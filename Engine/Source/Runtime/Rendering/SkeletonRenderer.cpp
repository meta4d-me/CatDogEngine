#include "SkeletonRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkinMeshComponent.h"
#include "RenderContext.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Window/Input.h"

namespace engine
{

namespace details
{

float CustomFMod(float dividend, float divisor)
{
	if (divisor == 0.0f)
	{
		return 0.0f;
	}

	int quotient = static_cast<int>(dividend / divisor);
	float result = dividend - static_cast<float>(quotient) * divisor;

	if (result == 0.0f && dividend != 0.0f)
	{
		result = 0.0f;
	}
	if ((dividend < 0 && divisor > 0) || (dividend > 0 && divisor < 0))
	{
		result = -result;
	}
	return result;
}

cd::Matrix4x4 CalculateRootBoneTransform(const cd::SceneDatabase* pSceneDatabase, float animationTime, const char* clipName)
{
	auto CalculateInterpolatedTranslation = [&animationTime](const cd::Track* pTrack) -> cd::Vec3f
	{
		if (1U == pTrack->GetTranslationKeyCount())
		{
			const auto& firstKey = pTrack->GetTranslationKeys()[0];
			return firstKey.GetValue();
		}

		for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetTranslationKeyCount() - 1; ++keyIndex)
		{
			const auto& nextKey = pTrack->GetTranslationKeys()[keyIndex + 1];
			if (animationTime < nextKey.GetTime())
			{
				const auto& currentKey = pTrack->GetTranslationKeys()[keyIndex];
				float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
				float keyFrameRate = (animationTime - currentKey.GetTime()) / keyFrameDeltaTime;
				assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

				return cd::Vec3f::Lerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate);
			}
		}
		const auto& Key = pTrack->GetTranslationKeys()[pTrack->GetTranslationKeyCount() - 1];
		return Key.GetValue();
	};

	auto CalculateInterpolatedRotation = [&animationTime](const cd::Track* pTrack) -> cd::Quaternion
	{
		/*	if (1U == pTrack->GetRotationKeyCount())
			{
				const auto& firstKey = pTrack->GetRotationKeys()[0];
				return firstKey.GetValue();
			}

			for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetRotationKeyCount() - 1; ++keyIndex)
			{
				const auto& nextKey = pTrack->GetRotationKeys()[keyIndex + 1];
				if (animationTime < nextKey.GetTime())
				{
					const auto& currentKey = pTrack->GetRotationKeys()[keyIndex];
					float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
					float keyFrameRate = (animationTime - currentKey.GetTime()) / keyFrameDeltaTime;
					assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

					return cd::Quaternion::SLerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate).Normalize();
				}
			}*/

		return cd::Quaternion::Identity();
	};

	auto CalculateInterpolatedScale = [&animationTime](const cd::Track* pTrack) -> cd::Vec3f
	{
		if (1U == pTrack->GetScaleKeyCount())
		{
			const auto& firstKey = pTrack->GetScaleKeys()[0];
			return firstKey.GetValue();
		}

		for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetScaleKeyCount() - 1; ++keyIndex)
		{
			const auto& nextKey = pTrack->GetScaleKeys()[keyIndex + 1];
			if (animationTime < nextKey.GetTime())
			{
				const auto& currentKey = pTrack->GetScaleKeys()[keyIndex];
				float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
				float keyFrameRate = (animationTime - currentKey.GetTime()) / keyFrameDeltaTime;
				assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

				return cd::Vec3f::Lerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate);
			}
		}

		return cd::Vec3f::One();
	};

	cd::Matrix4x4 boneLocalTransform = cd::Matrix4x4::Identity();
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName((std::string(clipName) + pSceneDatabase->GetBone(0).GetName()).c_str()))
	{
		boneLocalTransform = cd::Transform(CalculateInterpolatedTranslation(pTrack),
			CalculateInterpolatedRotation(pTrack),
			CalculateInterpolatedScale(pTrack)).GetMatrix() * pSceneDatabase->GetBone(0).GetOffset();;
	}

	return boneLocalTransform;
}

//Get current character's move dirction and speed,and steering speed. 0 < time < duration
cd::Vec3f GetRootMovementSpeed(const cd::Track* pTrack, float time, const cd::SceneDatabase* pSceneDatabase)
{
	cd::Vec3f rootMovementSpeed = cd::Vec3f::Zero();
	if (1U == pTrack->GetTranslationKeyCount())
	{
		return rootMovementSpeed;
	}
	for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetTranslationKeyCount() - 1; ++keyIndex)
	{
		const auto& nextTranslateKey = pTrack->GetTranslationKeys()[keyIndex + 1];
		if (time < nextTranslateKey.GetTime())
		{
			const auto& currentKey = pTrack->GetTranslationKeys()[keyIndex];
			rootMovementSpeed = (nextTranslateKey.GetValue() - currentKey.GetValue()) / (nextTranslateKey.GetTime() - currentKey.GetTime());
			const auto& a = pTrack->GetTranslationKeys()[pTrack->GetTranslationKeyCount() - 1];
			const auto& b = pTrack->GetTranslationKeys()[0];
			return rootMovementSpeed;
		}
	}
	return rootMovementSpeed;
}

cd::Vec3f GetRootRotationSpeed(const cd::Track* pTrack, float time, const cd::SceneDatabase* pSceneDatabase)
{
	cd::Vec3f rootRotationSpeed = cd::Vec3f::Zero();
	if (1U == pTrack->GetRotationKeyCount())
	{
		return rootRotationSpeed;
	}
	for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetTranslationKeyCount() - 1; ++keyIndex)
	{
		const auto& nextRotationKey = pTrack->GetRotationKeys()[keyIndex + 1];
		if (time < nextRotationKey.GetTime())
		{
			const auto& curRotationKey = pTrack->GetRotationKeys()[keyIndex];
			cd::Vec3f nextRotation(nextRotationKey.GetValue().Pitch(), nextRotationKey.GetValue().Yaw(), nextRotationKey.GetValue().Roll());
			cd::Vec3f curRotation(curRotationKey.GetValue().Pitch(), curRotationKey.GetValue().Yaw(), curRotationKey.GetValue().Roll());
			rootRotationSpeed = (nextRotation - curRotation) / (nextRotationKey.GetTime() - curRotationKey.GetTime());
			const auto& a = pTrack->GetRotationKeys()[0];
			const auto& b = pTrack->GetRotationKeys()[pTrack->GetTranslationKeyCount() - 1];
			cd::Vec3f n(b.GetValue().Pitch(), b.GetValue().Yaw(), b.GetValue().Roll());
			cd::Vec3f c(a.GetValue().Pitch(), a.GetValue().Yaw(), a.GetValue().Roll());
			cd::Vec3f Speed = (n - c) / (b.GetTime() - a.GetTime());
			return rootRotationSpeed;
		}
	}

	return rootRotationSpeed;
}

void CalculateTransform(std::vector<cd::Matrix4x4>& boneMatrices, const cd::SceneDatabase* pSceneDatabase,
	float animationTime, const cd::Bone& bone, const char* clipName, SkinMeshComponent* pSkinmeshConponent, cd::Matrix4x4& curGlobalDeltaMatrix, float deltaTime, bool isPlaying)
{
	auto CalculateInterpolatedTranslation = [&animationTime](const cd::Track* pTrack) -> cd::Vec3f
	{
		if (1U == pTrack->GetTranslationKeyCount())
		{
			const auto& firstKey = pTrack->GetTranslationKeys()[0];
			return firstKey.GetValue();
		}

		for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetTranslationKeyCount() - 1; ++keyIndex)
		{
			const auto& nextKey = pTrack->GetTranslationKeys()[keyIndex + 1];
			if (animationTime <= nextKey.GetTime())
			{
				const auto& currentKey = pTrack->GetTranslationKeys()[keyIndex];
				float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
				float keyFrameRate = (animationTime - currentKey.GetTime()) / keyFrameDeltaTime;
				assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

				return cd::Vec3f::Lerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate);
			}

		}
		const auto& Key = pTrack->GetTranslationKeys()[pTrack->GetTranslationKeyCount() - 1];
		return Key.GetValue();
	};

	auto CalculateInterpolatedRotation = [&animationTime](const cd::Track* pTrack) -> cd::Quaternion
	{
		if (1U == pTrack->GetRotationKeyCount())
		{
			const auto& firstKey = pTrack->GetRotationKeys()[0];
			return firstKey.GetValue();
		}

		for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetRotationKeyCount() - 1; ++keyIndex)
		{
			const auto& nextKey = pTrack->GetRotationKeys()[keyIndex + 1];
			if (animationTime <= nextKey.GetTime())
			{
				const auto& currentKey = pTrack->GetRotationKeys()[keyIndex];
				float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
				float keyFrameRate = (animationTime - currentKey.GetTime()) / keyFrameDeltaTime;
				assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

				return cd::Quaternion::SLerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate).Normalize();
			}
		}
		const auto& Key = pTrack->GetRotationKeys()[pTrack->GetTranslationKeyCount() - 1];
		return Key.GetValue();
	};

	auto CalculateInterpolatedScale = [&animationTime](const cd::Track* pTrack) -> cd::Vec3f
	{
		if (1U == pTrack->GetScaleKeyCount())
		{
			const auto& firstKey = pTrack->GetScaleKeys()[0];
			return firstKey.GetValue();
		}

		for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetScaleKeyCount() - 1; ++keyIndex)
		{
			const auto& nextKey = pTrack->GetScaleKeys()[keyIndex + 1];
			if (animationTime <= nextKey.GetTime())
			{
				const auto& currentKey = pTrack->GetScaleKeys()[keyIndex];
				float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
				float keyFrameRate = (animationTime - currentKey.GetTime()) / keyFrameDeltaTime;
				assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

				return cd::Vec3f::Lerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate);
			}
		}

		return cd::Vec3f::One();
	};

	static cd::Matrix4x4 deltaGlobalTransform = cd::Matrix4x4::Identity();
	static cd::Vec3f totalTranslate = cd::Vec3f::Zero();
	static cd::Vec3f totalRotaion = cd::Vec3f::Zero();
	cd::Matrix4x4 curBoneGlobalMatrix = cd::Matrix4x4::Identity();
	static float time = 0.0f;
	static cd::Matrix4x4 total = cd::Matrix4x4::Identity();
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName((std::string(clipName) + bone.GetName()).c_str()))
	{
		if (0 == bone.GetID().Data())
		{

			cd::Vec3f translate = cd::Vec3f::Zero();
			cd::Vec3f rotation = cd::Vec3f::Zero();
			if (isPlaying)
			{
				cd::Vec3f rootMoveSpeed = GetRootMovementSpeed(pTrack, animationTime, pSceneDatabase);
				cd::Vec3f rootRotationSpeed = GetRootRotationSpeed(pTrack, animationTime, pSceneDatabase);
				translate = rootMoveSpeed * (deltaTime);
				totalTranslate += translate;
				rotation = rootRotationSpeed * deltaTime;
				totalRotaion += rotation;
				const auto& srcTranslation = pTrack->GetTranslationKeys()[0];
				const auto& srcRotation = pTrack->GetRotationKeys()[0];
				const auto& srcSale = pTrack->GetScaleKeys()[0];
				cd::Matrix4x4 srcTransform = cd::Transform(srcTranslation.GetValue(), srcRotation.GetValue(), srcSale.GetValue()).GetMatrix();
				//CD_INFO(totalrotation);
				//cd::Quaternion::FromPitchYawRoll(rotation.x(),rotation.y(),rotation.z())
				cd::Vec3f globalTranslate = srcTranslation.GetValue() + totalTranslate;
				cd::Vec3f srcRotionVec3(srcRotation.GetValue().Pitch(), srcRotation.GetValue().Yaw(), srcRotation.GetValue().Roll());
				cd::Vec3f globalRotation = srcRotionVec3 + totalRotaion;
				cd::Quaternion globalRot = cd::Quaternion::FromPitchYawRoll(rotation.x(), rotation.y(), rotation.z()) * srcRotation.GetValue();
				cd::Matrix4x4 globalTransform = cd::Transform(globalTranslate, globalRot, cd::Vec3f::One()).GetMatrix();
				//deltaGlobalTransform = globalTransform * deltaGlobalTransform;
				curBoneGlobalMatrix = globalTransform;
				pSkinmeshConponent->SetRootMatrix(curBoneGlobalMatrix);
				pSkinmeshConponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
				pSkinmeshConponent->GetBoneGlobalMatrix(bone.GetID().Data());
				if (animationTime >= time)
				{
					//cd::Vec3f rootMoveSpeed = GetRootMovementSpeed(pTrack, animationTime, pSceneDatabase);
					//cd::Vec3f rootRotationSpeed = GetRootRotationSpeed(pTrack, animationTime, pSceneDatabase);
					deltaGlobalTransform = cd::Transform(CalculateInterpolatedTranslation(pTrack),
						CalculateInterpolatedRotation(pTrack),
						CalculateInterpolatedScale(pTrack)).GetMatrix();
					curBoneGlobalMatrix = total * deltaGlobalTransform;
					pSkinmeshConponent->SetRootMatrix(curBoneGlobalMatrix);
					pSkinmeshConponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);

				}
				else
				{
					const auto& translation = pTrack->GetTranslationKeys()[pTrack->GetTranslationKeyCount() - 1];
					const auto& rotation = pTrack->GetRotationKeys()[pTrack->GetTranslationKeyCount() - 1];
					const auto& scale = pTrack->GetScaleKeys()[pTrack->GetTranslationKeyCount() - 1];
					//pTrack->GetTranslationKey(pTrack->GetTranslationKeyCount() - 1); 
					const auto& srcTranslation = pTrack->GetTranslationKeys()[0];
					const auto& srcRotation = pTrack->GetRotationKeys()[0];
					const auto& srcSale = pTrack->GetScaleKeys()[0];
					cd::Matrix4x4 srcTransform = cd::Transform(srcTranslation.GetValue(), srcRotation.GetValue(), srcSale.GetValue()).GetMatrix();
					cd::Matrix4x4 lastTransform = cd::Transform(translation.GetValue(), rotation.GetValue(), scale.GetValue()).GetMatrix() * srcTransform.Inverse();
					total = lastTransform * total;
					deltaGlobalTransform = cd::Transform(CalculateInterpolatedTranslation(pTrack),
						CalculateInterpolatedRotation(pTrack),
						CalculateInterpolatedScale(pTrack)).GetMatrix();
					curBoneGlobalMatrix = total * deltaGlobalTransform;
					pSkinmeshConponent->SetRootMatrix(curBoneGlobalMatrix);
					pSkinmeshConponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
					curBoneGlobalMatrix = pSkinmeshConponent->GetRootMatrix();
				}
				time = animationTime;
			}
			else
			{
				/*deltaGlobalTransform = pSkinmeshConponent->GetRootMatrix().Inverse() * cd::Transform(CalculateInterpolatedTranslation(pTrack),
					CalculateInterpolatedRotation(pTrack),
					CalculateInterpolatedScale(pTrack)).GetMatrix();*/
				curBoneGlobalMatrix = pSkinmeshConponent->GetBoneGlobalMatrix(bone.GetID().Data());
				pSkinmeshConponent->SetRootMatrix(curBoneGlobalMatrix);
				pSkinmeshConponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
			}
		}
		else
		{
			curBoneGlobalMatrix = pSkinmeshConponent->GetBoneGlobalMatrix(bone.GetParentID().Data()) * cd::Transform(CalculateInterpolatedTranslation(pTrack),
				CalculateInterpolatedRotation(pTrack),
				CalculateInterpolatedScale(pTrack)).GetMatrix();
			pSkinmeshConponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
		}
	}

	boneMatrices[bone.GetID().Data()] = curBoneGlobalMatrix * bone.GetOffset();

	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		CalculateTransform(boneMatrices, pSceneDatabase, animationTime, childBone, clipName, pSkinmeshConponent, curGlobalDeltaMatrix, deltaTime, isPlaying);
	}
}

cd::Matrix4x4 CalculateInterpolationTransform(const cd::Track* pTrack, const float time)
{
	cd::Vec3f localTranslate = cd::Vec3f::Zero();
	cd::Quaternion localRotation = cd::Quaternion::Identity();
	cd::Vec3f localScale = cd::Vec3f::One();
	if (1U == pTrack->GetTranslationKeyCount())
	{
		const auto& firstKey = pTrack->GetTranslationKeys()[0];
		localTranslate = firstKey.GetValue();
	}
	for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetTranslationKeyCount() - 1; ++keyIndex)
	{
		const auto& nextKey = pTrack->GetTranslationKeys()[keyIndex + 1];
		if (time <= nextKey.GetTime())
		{
			const auto& currentKey = pTrack->GetTranslationKeys()[keyIndex];
			float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
			float keyFrameRate = (time - currentKey.GetTime()) / keyFrameDeltaTime;
			assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

			localTranslate = cd::Vec3f::Lerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate);
			break;
		}
		const auto& translateKey = pTrack->GetTranslationKeys()[pTrack->GetTranslationKeyCount() - 1];
		localTranslate = translateKey.GetValue();
	}


	if (1U == pTrack->GetRotationKeyCount())
	{
		const auto& firstKey = pTrack->GetRotationKeys()[0];
		localRotation = firstKey.GetValue();
	}

	for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetRotationKeyCount() - 1; ++keyIndex)
	{
		const auto& nextKey = pTrack->GetRotationKeys()[keyIndex + 1];
		if (time <= nextKey.GetTime())
		{
			const auto& currentKey = pTrack->GetRotationKeys()[keyIndex];
			float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
			float keyFrameRate = (time - currentKey.GetTime()) / keyFrameDeltaTime;
			assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

			localRotation = cd::Quaternion::SLerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate).Normalize();
			break;
		}
		const auto& rotationKey = pTrack->GetRotationKeys()[pTrack->GetTranslationKeyCount() - 1];
		localRotation = rotationKey.GetValue();
	}


	if (1U == pTrack->GetScaleKeyCount())
	{
		const auto& firstKey = pTrack->GetScaleKeys()[0];
		localScale = firstKey.GetValue();
	}

	for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetScaleKeyCount() - 1; ++keyIndex)
	{
		const auto& nextKey = pTrack->GetScaleKeys()[keyIndex + 1];
		if (time <= nextKey.GetTime())
		{
			const auto& currentKey = pTrack->GetScaleKeys()[keyIndex];
			float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
			float keyFrameRate = (time - currentKey.GetTime()) / keyFrameDeltaTime;
			assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

			localScale = cd::Vec3f::Lerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate);
			break;
		}
		localScale = cd::Vec3f::One();

	}
	return cd::Transform(localTranslate, localRotation, localScale).GetMatrix();
}

void BlendTwoPos(std::vector<cd::Matrix4x4>& boneMatrices, const cd::SceneDatabase* pSceneDatabase,
	float blendTimeProgress, const cd::Bone& bone, const char* clipAName, const char* clipBName, SkinMeshComponent* pSkinmeshConponent, cd::Matrix4x4& curGlobalDeltaMatrix, const cd::Matrix4x4 rootBoonTransform, const float factor)
{
	cd::Matrix4x4 localTransformA = cd::Matrix4x4::Identity();
	cd::Matrix4x4 localTransformB = cd::Matrix4x4::Identity();
	cd::Matrix4x4 curBoneGlobalMatrix = cd::Matrix4x4::Identity();
	float clipATime = pSceneDatabase->GetAnimation(0).GetDuration() * blendTimeProgress;
	float clipBTime = pSceneDatabase->GetAnimation(1).GetDuration() * blendTimeProgress;
	static float progress = 0.0f;
	static cd::Matrix4x4 total = cd::Matrix4x4::Identity();
	static cd::Matrix4x4 lastMatrix = cd::Matrix4x4::Identity();
	const cd::Track* pTrackA = pSceneDatabase->GetTrackByName((std::string(clipAName) + bone.GetName()).c_str());
	if (const cd::Track* pTrackA = pSceneDatabase->GetTrackByName((std::string(clipAName) + bone.GetName()).c_str()))
	{
		localTransformA = CalculateInterpolationTransform(pTrackA, clipATime);
	}
	if (const cd::Track* pTrackB = pSceneDatabase->GetTrackByName((std::string(clipBName) + bone.GetName()).c_str()))
	{
		localTransformB = CalculateInterpolationTransform(pTrackB, clipBTime);
	}

	cd::Vec3f translationBlend = cd::Vec3f::Lerp(localTransformA.GetTranslation(), localTransformB.GetTranslation(), factor);
	cd::Quaternion rotationBlend = cd::Quaternion::SLerp(cd::Quaternion::FromMatrix(localTransformA.GetRotation()), cd::Quaternion::FromMatrix(localTransformB.GetRotation()), factor);
	cd::Vec3f scaleBlend = cd::Vec3f::Lerp(localTransformA.GetScale(), localTransformB.GetScale(), factor);
	cd::Matrix4x4 matrixBlend = cd::Transform(translationBlend, rotationBlend, scaleBlend).GetMatrix();
	cd::Matrix4x4 deltaGlobalTransform = cd::Matrix4x4::Identity();
	if (0 == bone.GetID().Data())
	{
		//if (blendTimeProgress >= progress)
		{

			curBoneGlobalMatrix = total * matrixBlend;

			pSkinmeshConponent->SetRootMatrix(curBoneGlobalMatrix);
			pSkinmeshConponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
			lastMatrix = matrixBlend;

		}
		/*else
		{
			const auto& srcTranslation = pTrackA->GetTranslationKeys()[0];
			const auto& srcRotation = pTrackA->GetRotationKeys()[0];
			const auto& srcSale = pTrackA->GetScaleKeys()[0];
			cd::Matrix4x4 srcTransform = cd::Transform(srcTranslation.GetValue(), srcRotation.GetValue(), srcSale.GetValue()).GetMatrix();
			cd::Matrix4x4 deltaTransform = pSkinmeshConponent->GetRootMatrix() * srcTransform.Inverse();
			total = deltaTransform;
			deltaTransform = matrixBlend * srcTransform.Inverse();
			total = deltaTransform * total;
			curBoneGlobalMatrix = total * pSkinmeshConponent->GetRootMatrix();
			pSkinmeshConponent->SetRootMatrix(curBoneGlobalMatrix);
			pSkinmeshConponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
		}*/
		progress = blendTimeProgress;
	}
	else
	{
		curBoneGlobalMatrix = pSkinmeshConponent->GetBoneGlobalMatrix(bone.GetParentID().Data()) * matrixBlend;
		pSkinmeshConponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
	}
	boneMatrices[bone.GetID().Data()] = curBoneGlobalMatrix * bone.GetOffset();
	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		BlendTwoPos(boneMatrices, pSceneDatabase, blendTimeProgress, childBone, clipAName, clipBName, pSkinmeshConponent, curGlobalDeltaMatrix, rootBoonTransform, factor);
	}
}

}

void SkeletonRenderer::Init()
{
	constexpr StringCrc programCrc = StringCrc("SkeletonProgram");
	GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_skeleton", "fs_AABB" });

	bgfx::setViewName(GetViewID(), "SkeletonRenderer");
}

void SkeletonRenderer::Warmup()
{
	GetRenderContext()->UploadShaderProgram("SkeletonProgram");
}

void SkeletonRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void SkeletonRenderer::Render(float deltaTime)
{
	static float animationRunningTime = 0.0f;
	static float playTime = 0.0f;//for replay one animation
	static cd::Matrix4x4 rootTransform = cd::Matrix4x4::Identity();
	static cd::Matrix4x4 deltaRootTransform = cd::Matrix4x4::Identity();
	cd::Matrix4x4 beforRootTransform = cd::Matrix4x4::Identity();
	cd::Matrix4x4 nowRootTransform = cd::Matrix4x4::Identity();
	const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
	for (Entity entity : m_pCurrentSceneWorld->GetSkinMeshEntities())
	{
		auto pSkinMeshComponent = m_pCurrentSceneWorld->GetSkinMeshComponent(entity);
		if (!pSkinMeshComponent)
		{
			continue;
		}

		AnimationComponent* pAnimationComponent = m_pCurrentSceneWorld->GetAnimationComponent(entity);
		TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity);
		const char* clipName;
		float ticksPerSecond = 0.0f;
		float duration = 0.0f;
		bool isLoopOut = false;
		if (pAnimationComponent && pAnimationComponent->GetIsPlaying())
		{
			animationRunningTime += deltaTime * pAnimationComponent->GetPlayBackSpeed();
		}
		static std::vector<cd::Matrix4x4> globalDeltaBoneMatrix;
		static std::vector<cd::Matrix4x4> boneMatrixA;
		static std::vector<cd::Matrix4x4> boneMatrixB;
		globalDeltaBoneMatrix.clear();
		boneMatrixA.clear();
		boneMatrixB.clear();
		for (uint16_t boneIndex = 0; boneIndex < 128; ++boneIndex)
		{
			globalDeltaBoneMatrix.push_back(cd::Matrix4x4::Identity());
			boneMatrixA.push_back(cd::Matrix4x4::Identity());
			boneMatrixB.push_back(cd::Matrix4x4::Identity());
		}
		if (engine::AnimationClip::Idel == pAnimationComponent->GetAnimationClip())
		{
			cd::Matrix4x4 rootBone = cd::Matrix4x4::Identity();
			const cd::Animation& pAnimation = pSceneDatabase->GetAnimation(0);
			clipName = pSceneDatabase->GetAnimation(0).GetName();
			duration = pSceneDatabase->GetAnimation(0).GetDuration();
			ticksPerSecond = pSceneDatabase->GetAnimation(0).GetTicksPerSecnod();

			float animationTime = details::CustomFMod(animationRunningTime, duration);
			CD_INFO(animationTime);
			pAnimationComponent->SetAnimationPlayTime(animationTime);
			cd::Matrix4x4 curBoneGlobalMatrix = pSkinMeshComponent->GetBoneGlobalMatrix(0);
			cd::Matrix4x4 curGlobalDeltaMatrix = cd::Matrix4x4::Identity();
			//if (animationTime >= playTime)
			{
				nowRootTransform = details::CalculateRootBoneTransform(pSceneDatabase, animationTime, clipName);
				playTime = animationTime;
				details::CalculateTransform(globalDeltaBoneMatrix, pSceneDatabase, animationTime, pSceneDatabase->GetBone(0), clipName, pSkinMeshComponent, curGlobalDeltaMatrix, deltaTime * pAnimationComponent->GetPlayBackSpeed(), pAnimationComponent->GetIsPlaying());
			}
			/*	else
				{
					beforRootTransform = details::CalculateRootBoneTransform(pSceneDatabase, duration, clipName);
					CD_INFO("loopTime");
					CD_INFO(playTime);
					deltaRootTransform = beforRootTransform * deltaRootTransform;
					playTime = animationTime;
					details::CalculateTransform(globalDeltaBoneMatrix, pSceneDatabase, duration, pSceneDatabase->GetBone(0), clipName, pSkinMeshComponent, curBoneGlobalMatrix, deltaRootTransform);
				}*/
				//pTransformComponent->SetTransform(cd::Transform(rootTransform.GetTranslation(), cd::Quaternion::FromMatrix(rootTransform.GetRotation()), rootTransform.GetScale()));
				//details::CalculateTransform(globalDeltaBoneMatrix, pSceneDatabase, animationTime, pSceneDatabase->GetBone(0), clipName, pSkinMeshComponent, curBoneGlobalMatrix, deltaRootTransform);

		}
		else if (engine::AnimationClip::Walking == pAnimationComponent->GetAnimationClip())
		{
			const cd::Animation& pAnimation = pSceneDatabase->GetAnimation(1);
			clipName = pSceneDatabase->GetAnimation(1).GetName();
			duration = pSceneDatabase->GetAnimation(1).GetDuration();
			ticksPerSecond = pSceneDatabase->GetAnimation(1).GetTicksPerSecnod();
			float animationTime = details::CustomFMod(animationRunningTime, duration);
			cd::Matrix4x4 rootBoneInserve = details::CalculateRootBoneTransform(pSceneDatabase, animationTime, clipName);
			cd::Matrix4x4 curGlobalDeltaMatrix = cd::Matrix4x4::Identity();
			details::CalculateTransform(globalDeltaBoneMatrix, pSceneDatabase, animationTime, pSceneDatabase->GetBone(0), clipName, pSkinMeshComponent, curGlobalDeltaMatrix, deltaTime, pAnimationComponent->GetIsPlaying());
		}
		else if (engine::AnimationClip::Running == pAnimationComponent->GetAnimationClip())
		{
			const cd::Animation& pAnimation = pSceneDatabase->GetAnimation(2);
			clipName = pSceneDatabase->GetAnimation(2).GetName();
			duration = pSceneDatabase->GetAnimation(2).GetDuration();
			ticksPerSecond = pSceneDatabase->GetAnimation(1).GetTicksPerSecnod();
			float animationTime = details::CustomFMod(animationRunningTime, duration);
			cd::Matrix4x4 rootBoneInserve = details::CalculateRootBoneTransform(pSceneDatabase, animationTime, clipName);
			//details::CalculateTransform(globalDeltaBoneMatrix, pSceneDatabase, animationTime, pSceneDatabase->GetBone(0), clipName, pSkinMeshComponent, curBoneGlobalMatrix, cd::Matrix4x4::Identity());
		}
		else if (engine::AnimationClip::Blend == pAnimationComponent->GetAnimationClip())
		{
			float factor = pAnimationComponent->GetBlendFactor();
			float clipATime = pSceneDatabase->GetAnimation(0).GetDuration();
			float clipBTime = pSceneDatabase->GetAnimation(1).GetDuration();

			float clipARunningTime = details::CustomFMod(animationRunningTime, clipATime);
			float clipBRunningTime = details::CustomFMod(animationRunningTime, clipBTime);
			const float blendSpeed = clipATime + (clipBTime - clipATime) * factor;
			float clipASpeed = clipATime / blendSpeed;
			pAnimationComponent->SetPlayBackSpeed(clipASpeed);
			float clipBSpeed = clipBTime / blendSpeed;
			float clipAProgress = clipARunningTime / clipATime;

			cd::Matrix4x4 curGlobalDeltaMatrix = cd::Matrix4x4::Identity();
			details::BlendTwoPos(globalDeltaBoneMatrix, pSceneDatabase, clipAProgress, pSceneDatabase->GetBone(0), pSceneDatabase->GetAnimation(0).GetName(), pSceneDatabase->GetAnimation(1).GetName(), pSkinMeshComponent, curGlobalDeltaMatrix, deltaRootTransform, factor);
		}
		else if (engine::AnimationClip::Switch == pAnimationComponent->GetAnimationClip())
		{

		}


		//assert(ticksPerSecond > 1.0f);
		float animationTime = details::CustomFMod(animationRunningTime, duration);

		bgfx::setUniform(bgfx::UniformHandle{ pSkinMeshComponent->GetBoneMatrixsUniform() }, globalDeltaBoneMatrix.data(), static_cast<uint16_t>(globalDeltaBoneMatrix.size()));
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pSkinMeshComponent->GetBoneVBH() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pSkinMeshComponent->GetBoneIBH() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;

		bgfx::setState(state);
	
		GetRenderContext()->Submit(GetViewID(), "SkeletonProgram");

	}

}

}