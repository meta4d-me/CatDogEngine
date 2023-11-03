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

			return cd::Vec3f::Zero();
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
					if (animationTime < nextKey.GetTime())
					{
						const auto& currentKey = pTrack->GetRotationKeys()[keyIndex];
						float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
						float keyFrameRate = (animationTime - currentKey.GetTime()) / keyFrameDeltaTime;
						assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

						return cd::Quaternion::Lerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate).Normalize();
					}
				}

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
			CalculateInterpolatedScale(pTrack)).GetMatrix() * pSceneDatabase->GetBone(0).GetOffset();
	}

	return boneLocalTransform;
}

void CalculateTransform(std::vector<cd::Matrix4x4>& boneMatrices, const cd::SceneDatabase* pSceneDatabase,
	float animationTime, const cd::Bone& bone, const char* clipName, const cd::Matrix4x4 rootBoonTransform)
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

			return cd::Vec3f::Zero();
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
				if (animationTime < nextKey.GetTime())
				{
					const auto& currentKey = pTrack->GetRotationKeys()[keyIndex];
					float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
					float keyFrameRate = (animationTime - currentKey.GetTime()) / keyFrameDeltaTime;
					assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);

					return cd::Quaternion::Lerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate).Normalize();
				}
			}

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
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName((std::string(clipName) + bone.GetName()).c_str()))
	{
		boneLocalTransform = cd::Transform(CalculateInterpolatedTranslation(pTrack),
			CalculateInterpolatedRotation(pTrack),
			CalculateInterpolatedScale(pTrack)).GetMatrix() * bone.GetOffset();
	}

	boneMatrices[bone.GetID().Data()] = rootBoonTransform * boneLocalTransform ;

	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		CalculateTransform(boneMatrices, pSceneDatabase, animationTime, childBone, clipName, rootBoonTransform);
	}
}

void BlendTwoPos(std::vector<cd::Matrix4x4>& blendBoneMatrices, std::vector<cd::Matrix4x4>& BoneMatricesA, std::vector<cd::Matrix4x4>& BoneMatricesB, float animationRunningTime, float factor, const cd::SceneDatabase* pSceneDatabase)
{
	float clipATime = pSceneDatabase->GetAnimation(0).GetDuration();
	float clipBTime = pSceneDatabase->GetAnimation(1).GetDuration();
	const float blendTime = clipATime + (clipBTime - clipATime) * factor;
	float clipARunningTime = CustomFMod(animationRunningTime, clipATime);
	float clipBRunningTime = CustomFMod(animationRunningTime, clipBTime);
	float blendRunningTime = CustomFMod(animationRunningTime, blendTime);
	float blendTimeProgress = blendRunningTime / blendTime;

	CalculateTransform(BoneMatricesA, pSceneDatabase, clipATime * blendTimeProgress, pSceneDatabase->GetBone(0), pSceneDatabase->GetAnimation(0).GetName(), CalculateRootBoneTransform(pSceneDatabase, clipARunningTime, pSceneDatabase->GetAnimation(0).GetName()));
	CalculateTransform(BoneMatricesB, pSceneDatabase, clipBTime * blendTimeProgress, pSceneDatabase->GetBone(0), pSceneDatabase->GetAnimation(1).GetName(), CalculateRootBoneTransform(pSceneDatabase, clipBRunningTime, pSceneDatabase->GetAnimation(1).GetName()));
	uint32_t bonnCount = pSceneDatabase->GetBoneCount();
	for (auto& bone : pSceneDatabase->GetBones())
	{
		cd::Vec3f translationA = BoneMatricesA[bone.GetID().Data()].GetTranslation();
		cd::Quaternion rotationA = cd::Quaternion::FromMatrix(BoneMatricesA[bone.GetID().Data()].GetRotation());
		cd::Vec3f scaleA = BoneMatricesA[bone.GetID().Data()].GetScale();
		cd::Vec3f translationB = BoneMatricesB[bone.GetID().Data()].GetTranslation();
		cd::Quaternion rotationB = cd::Quaternion::FromMatrix(BoneMatricesB[bone.GetID().Data()].GetRotation());
		cd::Vec3f scaleB = BoneMatricesB[bone.GetID().Data()].GetScale();
		
		cd::Transform transA = cd::Transform(translationA, rotationA, scaleA);
		cd::Transform transB = cd::Transform(translationB, rotationB, scaleB);
		cd::Vec3f blendTranslation = cd::Vec3f::Lerp(transA.GetTranslation(), transB.GetTranslation(), factor);
		cd::Quaternion blendRotation = cd::Quaternion::SLerpNormalized(transA.GetRotation(), transB.GetRotation(), factor);
		cd::Vec3f blendScale = cd::Vec3f::Lerp(transA.GetScale(), transB.GetScale(), factor);

		cd::Matrix4x4 matrixA = BoneMatricesA[bone.GetID().Data()];
		cd::Matrix4x4 matrixB = BoneMatricesB[bone.GetID().Data()];
		cd::Vec4f col0 = matrixA.GetColumn(0) + (matrixB.GetColumn(0) - matrixA.GetColumn(0)) * factor;
		cd::Vec4f col1 = matrixA.GetColumn(1) + (matrixB.GetColumn(1) - matrixA.GetColumn(1)) * factor;
		cd::Vec4f col2 = matrixA.GetColumn(2) + (matrixB.GetColumn(2) - matrixA.GetColumn(2)) * factor;
		cd::Vec4f col3 = matrixA.GetColumn(3) + (matrixB.GetColumn(3) - matrixA.GetColumn(3)) * factor;
		cd::Matrix4x4 matrixBlend(col0,col1,col2,col3);
		cd::Matrix4x4 boneLocalTranform = cd::Transform(blendTranslation, blendRotation, blendScale).GetMatrix();
		blendBoneMatrices[bone.GetID().Data()] = boneLocalTranform;
	}
	

}

}

void SkeletonRenderer::Init()
{
	GetRenderContext()->CreateProgram("SkeletonProgram", "vs_skeleton.bin", "fs_AABB.bin");
	bgfx::setViewName(GetViewID(), "SkeletonRenderer");
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
			animationRunningTime += deltaTime;
		}
		static std::vector<cd::Matrix4x4> localBoneMatrix;
		static std::vector<cd::Matrix4x4> boneMatrixA;
		static std::vector<cd::Matrix4x4> boneMatrixB;
		localBoneMatrix.clear();
		boneMatrixA.clear();
		boneMatrixB.clear();
		for (uint16_t boneIndex = 0; boneIndex < 128; ++boneIndex)
		{
			localBoneMatrix.push_back(cd::Matrix4x4::Identity());
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
			pAnimationComponent->SetAnimationPlayTime(animationTime);
			if (animationTime >= playTime)
			{
				nowRootTransform = details::CalculateRootBoneTransform(pSceneDatabase, animationTime, clipName);
				playTime = animationTime;
			}
			else
			{
				beforRootTransform = details::CalculateRootBoneTransform(pSceneDatabase, playTime, clipName);
				nowRootTransform = details::CalculateRootBoneTransform(pSceneDatabase, animationTime, clipName);
				//nowRootTransform = nowRootTransform * beforRootTransform;
				deltaRootTransform = nowRootTransform * deltaRootTransform;
				playTime = animationTime;
			}
			//pTransformComponent->SetTransform(cd::Transform(rootTransform.GetTranslation(), cd::Quaternion::FromMatrix(rootTransform.GetRotation()), rootTransform.GetScale()));
			details::CalculateTransform(localBoneMatrix, pSceneDatabase, animationTime, pSceneDatabase->GetBone(0), clipName, nowRootTransform.Inverse());
		}
		else if (engine::AnimationClip::Walking == pAnimationComponent->GetAnimationClip())
		{
			const cd::Animation& pAnimation = pSceneDatabase->GetAnimation(1);
			clipName = pSceneDatabase->GetAnimation(1).GetName();
			duration = pSceneDatabase->GetAnimation(1).GetDuration();
			ticksPerSecond = pSceneDatabase->GetAnimation(1).GetTicksPerSecnod();
			float animationTime = details::CustomFMod(animationRunningTime, duration);
			cd::Matrix4x4 rootBoneInserve = details::CalculateRootBoneTransform(pSceneDatabase, animationTime, clipName);
			details::CalculateTransform(localBoneMatrix, pSceneDatabase, animationTime, pSceneDatabase->GetBone(0), clipName, rootBoneInserve);
		}
		else if (engine::AnimationClip::Running == pAnimationComponent->GetAnimationClip())
		{
			const cd::Animation& pAnimation = pSceneDatabase->GetAnimation(2);
			clipName = pSceneDatabase->GetAnimation(2).GetName();
			duration = pSceneDatabase->GetAnimation(2).GetDuration();
			ticksPerSecond = pSceneDatabase->GetAnimation(1).GetTicksPerSecnod();
			float animationTime = details::CustomFMod(animationRunningTime, duration);
			cd::Matrix4x4 rootBoneInserve = details::CalculateRootBoneTransform(pSceneDatabase, animationTime, clipName);
			details::CalculateTransform(localBoneMatrix, pSceneDatabase, animationTime, pSceneDatabase->GetBone(0), clipName, cd::Matrix4x4::Identity());
		}
		else if (engine::AnimationClip::Blend == pAnimationComponent->GetAnimationClip())
		{
			details::BlendTwoPos(localBoneMatrix, boneMatrixA, boneMatrixB, animationRunningTime, pAnimationComponent->GetBlendFactor(), pSceneDatabase);
		}

	
		assert(ticksPerSecond > 1.0f);
		float animationTime = details::CustomFMod(animationRunningTime, duration);

		bgfx::setUniform(bgfx::UniformHandle{ pSkinMeshComponent->GetBoneMatrixsUniform() }, localBoneMatrix.data(), static_cast<uint16_t>(localBoneMatrix.size()));
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pSkinMeshComponent->GetBoneVBH() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pSkinMeshComponent->GetBoneIBH() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;

		bgfx::setState(state);
		constexpr StringCrc SkeletonProgram("SkeletonProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(SkeletonProgram));

	}
	
}

}