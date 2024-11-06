#include "SkeletonRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkinMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Rendering/Resources/ShaderResource.h"

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

void CalculateTransform(std::vector<cd::Matrix4x4>& boneMatrices, const cd::SceneDatabase* pSceneDatabase,
	float animationTime, const cd::Bone& bone, const char* clipName, SkeletonComponent* pSkeletonConponent, cd::Matrix4x4& curGlobalDeltaMatrix, float deltaTime, bool isPlaying)
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
	static cd::Matrix4x4 transform = cd::Matrix4x4::Identity();

	cd::Matrix4x4 curBoneGlobalMatrix = cd::Matrix4x4::Identity();
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName((std::string(clipName) + bone.GetName()).c_str()))
	{
		if (0 == bone.GetID().Data())
		{
			
			transform = cd::Transform(CalculateInterpolatedTranslation(pTrack),
				CalculateInterpolatedRotation(pTrack),
				CalculateInterpolatedScale(pTrack)).GetMatrix();
			curBoneGlobalMatrix = transform;
			pSkeletonConponent->SetBoneGlobalMatrix(bone.GetID().Data(), transform);
			
		}
		else
		{
			curBoneGlobalMatrix = pSkeletonConponent->GetBoneGlobalMatrix(bone.GetParentID().Data()) * cd::Transform(CalculateInterpolatedTranslation(pTrack),
				CalculateInterpolatedRotation(pTrack),
				CalculateInterpolatedScale(pTrack)).GetMatrix();
			pSkeletonConponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
		}
	}

	boneMatrices[bone.GetID().Data()] = curBoneGlobalMatrix * bone.GetOffset();
	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		CalculateTransform(boneMatrices, pSceneDatabase, animationTime, childBone, clipName, pSkeletonConponent, curGlobalDeltaMatrix, deltaTime, isPlaying);
	}
}

cd::Vec3f CalculateBoneTranslate(const cd::Bone& bone, cd::Vec3f& translate, const cd::SceneDatabase* pSceneDatabase)
{
	const cd::Bone& parentBone = pSceneDatabase->GetBone(bone.GetParentID().Data());
	translate += parentBone.GetTransform().GetTranslation();
	if (0U != bone.GetParentID().Data())
	{
		CalculateBoneTranslate(parentBone, translate, pSceneDatabase);
	}
	return translate;
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
	float blendTimeProgress, const cd::Bone& bone, const char* clipAName, const char* clipBName, SkeletonComponent* pSkeletonComponent, cd::Matrix4x4& curGlobalDeltaMatrix, const cd::Matrix4x4 rootBoonTransform, const float factor)
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

			pSkeletonComponent->SetRootMatrix(curBoneGlobalMatrix);
			pSkeletonComponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
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
		curBoneGlobalMatrix = pSkeletonComponent->GetBoneGlobalMatrix(bone.GetParentID().Data()) * matrixBlend;
		pSkeletonComponent->SetBoneGlobalMatrix(bone.GetID().Data(), curBoneGlobalMatrix);
	}
	boneMatrices[bone.GetID().Data()] = curBoneGlobalMatrix * bone.GetOffset();
	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		BlendTwoPos(boneMatrices, pSceneDatabase, blendTimeProgress, childBone, clipAName, clipBName, pSkeletonComponent, curGlobalDeltaMatrix, rootBoonTransform, factor);
	}
}

}

void SkeletonRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("SkeletonProgram", "vs_skeleton", "fs_AABB"));
	bgfx::setViewName(GetViewID(), "SkeletonRenderer");
}

void SkeletonRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void SkeletonRenderer::Build()
{

}

void SkeletonRenderer::Render(float deltaTime)
{

	const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();

	static float animationRunningTime = 0.0f;
	static float playTime = 0.0f;//for replay one animation

	//const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	for (Entity entity : m_pCurrentSceneWorld->GetAnimationEntities())
	{
		auto pAnimationComponent = m_pCurrentSceneWorld->GetAnimationComponent(entity);
		if (!pAnimationComponent)
		{
			continue;
		}
		const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
		SkeletonComponent* pSkeletonComponent = m_pCurrentSceneWorld->GetSkeletonComponent(entity);
		const char* clipName;
		float ticksPerSecond = 0.0f;
		float duration = 0.0f;

		const SkeletonResource* pSkeletonResource = pSkeletonComponent->GetSkeletonResource();
		if (ResourceStatus::Ready != pSkeletonResource->GetStatus() &&
			ResourceStatus::Optimized != pSkeletonResource->GetStatus())
		{
			continue;
		}

		if (pAnimationComponent && pAnimationComponent->IsPlaying())
		{
			animationRunningTime += deltaTime * pAnimationComponent->GetPlayBackSpeed();
		}
		static cd::Matrix4x4 deltaRootTransform = cd::Matrix4x4::Identity();
		static std::vector<cd::Matrix4x4> globalDeltaBoneMatrix;
		static std::vector<cd::Matrix4x4> boneMatrixA;
		static std::vector<cd::Matrix4x4> boneMatrixB;
		globalDeltaBoneMatrix.clear();
		boneMatrixA.clear();
		boneMatrixB.clear();
		globalDeltaBoneMatrix.resize(128, cd::Matrix4x4::Identity());
		boneMatrixA.resize(128, cd::Matrix4x4::Identity());
		boneMatrixB.resize(128, cd::Matrix4x4::Identity());
		if (engine::AnimationClip::Idle == pAnimationComponent->GetAnimationClip())
		{
			cd::Matrix4x4 rootBone = cd::Matrix4x4::Identity();

			clipName = pSceneDatabase->GetAnimation(0).GetName();
			duration = pSceneDatabase->GetAnimation(0).GetDuration();
			ticksPerSecond = pSceneDatabase->GetAnimation(0).GetTicksPerSecond();

			float animationTime = details::CustomFMod(animationRunningTime, duration);
			pAnimationComponent->SetAnimationPlayTime(animationTime);
	
			cd::Matrix4x4 curGlobalDeltaMatrix = cd::Matrix4x4::Identity();
			details::CalculateTransform(globalDeltaBoneMatrix, pSceneDatabase, animationTime, pSceneDatabase->GetBone(0), clipName, pSkeletonComponent, curGlobalDeltaMatrix, deltaTime * pAnimationComponent->GetPlayBackSpeed(), pAnimationComponent->IsPlaying());
			
		}
		else if (engine::AnimationClip::Walking == pAnimationComponent->GetAnimationClip())
		{
			const cd::Animation& pAnimation = pSceneDatabase->GetAnimation(1);
			clipName = pSceneDatabase->GetAnimation(1).GetName();
			duration = pSceneDatabase->GetAnimation(1).GetDuration();
			ticksPerSecond = pSceneDatabase->GetAnimation(1).GetTicksPerSecond();

			float animationTime = details::CustomFMod(animationRunningTime, duration);
			pAnimationComponent->SetAnimationPlayTime(animationTime);

			cd::Matrix4x4 curGlobalDeltaMatrix = cd::Matrix4x4::Identity();
			details::CalculateTransform(globalDeltaBoneMatrix, pSceneDatabase, animationTime, pSceneDatabase->GetBone(0), clipName, pSkeletonComponent, curGlobalDeltaMatrix, deltaTime * pAnimationComponent->GetPlayBackSpeed(), pAnimationComponent->IsPlaying());
		}
		else if (engine::AnimationClip::Running == pAnimationComponent->GetAnimationClip())
		{
			const cd::Animation& pAnimation = pSceneDatabase->GetAnimation(2);
			clipName = pSceneDatabase->GetAnimation(2).GetName();
			duration = pSceneDatabase->GetAnimation(2).GetDuration();
			ticksPerSecond = pSceneDatabase->GetAnimation(1).GetTicksPerSecond();
			float animationTime = details::CustomFMod(animationRunningTime, duration);
	
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
			details::BlendTwoPos(globalDeltaBoneMatrix, pSceneDatabase, clipAProgress, pSceneDatabase->GetBone(0), pSceneDatabase->GetAnimation(0).GetName(), pSceneDatabase->GetAnimation(1).GetName(), pSkeletonComponent, curGlobalDeltaMatrix, deltaRootTransform, factor);
		}

		bgfx::setTransform(cd::Matrix4x4::Identity().begin());

		bgfx::setUniform(bgfx::UniformHandle{ pAnimationComponent->GetBoneMatrixsUniform() }, globalDeltaBoneMatrix.data(), static_cast<uint16_t>(globalDeltaBoneMatrix.size()));
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pSkeletonComponent->GetSkeletonResource()->GetVertexBufferHandle()});
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pSkeletonComponent->GetSkeletonResource()->GetIndexBufferHandle() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;

		bgfx::setState(state);

		constexpr StringCrc programHandleIndex{ "SkeletonProgram" };

		//GetRenderContext()->Submit(GetViewID(), programHandleIndex);
	}
}
}