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

void CalculateTransform(std::vector<cd::Matrix4x4>& boneMatrices, const cd::SceneDatabase* pSceneDatabase,
	float animationTime, const cd::Bone& bone)
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
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName(bone.GetName()))
	{
		boneLocalTransform = cd::Transform(CalculateInterpolatedTranslation(pTrack),
			CalculateInterpolatedRotation(pTrack),
			CalculateInterpolatedScale(pTrack)).GetMatrix() * bone.GetOffset();
	}

	boneMatrices[bone.GetID().Data()] = boneLocalTransform;

	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		CalculateTransform(boneMatrices, pSceneDatabase, animationTime, childBone);
	}
}

void CalculateBoneDeltaTransform(const cd::Bone& bone,uint32_t& time, const cd::SceneDatabase* pSceneDatabase, engine::SkinMeshComponent* pSkinMeshComponent,  cd::Matrix4x4& parentBoneTransform)
{
	cd::Matrix4x4 boneLocalTransform = bone.GetTransform().GetMatrix();
	cd::Matrix4x4 parentWorldMatrix = cd::Matrix4x4::Identity();
	cd::Quaternion parentRotationInserve = cd::Quaternion::Identity();
	if (bone.GetID().Data() != 0)
	{
		const cd::Bone& parentBone = pSceneDatabase->GetBone(bone.GetParentID().Data());
		parentWorldMatrix = parentBone.GetOffset().Inverse();
		parentRotationInserve = cd::Quaternion::FromMatrix(parentBone.GetOffset().GetRotation());

	}
	cd::Matrix4x4 globalTransform = parentWorldMatrix * boneLocalTransform;
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName(bone.GetName()))
	{
		//for (uint32_t index = 0; index < pTrack->GetTranslationKeyCount(); index++)
		//{
		if (time < pTrack->GetRotationKeyCount())
		{
			const char* name = bone.GetName();
			const auto& transKey = pTrack->GetTranslationKeys()[1];
			cd::Vec3f translation = transKey.GetValue();
		
			
			const auto& rotationKey= pTrack->GetRotationKeys()[1];
			cd::Quaternion rotation = rotationKey.GetValue();

			cd::Vec3f sourceTranslation = bone.GetTransform().GetTranslation();
			cd::Vec3f deltaTranslation = translation - sourceTranslation;

			cd::Quaternion sourcerotation = bone.GetTransform().GetRotation();
			cd::Quaternion deltaRotation = rotation * sourcerotation;
			deltaRotation = deltaRotation * parentRotationInserve;

			cd::Transform changeTransform = cd::Transform(deltaTranslation, deltaRotation, cd::Vec3f(1.0f, 1.0f, 1.0f));
			cd::Matrix4x4 globalTransform = parentBoneTransform * changeTransform.GetMatrix();
			pSkinMeshComponent->SetBoneChangeMatrix(bone.GetID().Data(), globalTransform);
			parentBoneTransform = globalTransform;

		}
		else
		{
			time = 0;
		}


	}


}

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

void SkeletonRenderer::Render(float delataTime)
{
	static float animationRunningTime = 0.0f;
	animationRunningTime += delataTime / 20.0f;
	const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
	for (Entity entity : m_pCurrentSceneWorld->GetAnimationEntities())
	{
		auto pSkinMeshComponent = m_pCurrentSceneWorld->GetSkinMeshComponent(entity);
		if (!pSkinMeshComponent)
		{
			continue;
		}

		AnimationComponent* pAnimationComponent = m_pCurrentSceneWorld->GetAnimationComponent(entity);
		if (!pAnimationComponent)
		{
			continue;
		}
		const cd::Animation* pAnimation = pAnimationComponent->GetAnimationData();
		float ticksPerSecond = pAnimation->GetTicksPerSecnod();
		assert(ticksPerSecond > 1.0f);
		float animationTime = details::CustomFMod(animationRunningTime * ticksPerSecond, pAnimation->GetDuration());

		static std::vector<cd::Matrix4x4> boneMatrices;
		boneMatrices.clear();
		for (uint16_t boneIndex = 0; boneIndex < 128; ++boneIndex)
		{
			boneMatrices.push_back(cd::Matrix4x4::Identity());
		}
		const cd::Bone& rootBone = pSceneDatabase->GetBone(0);
		details::CalculateTransform(boneMatrices, pSceneDatabase, animationTime, rootBone);

		bgfx::setUniform(bgfx::UniformHandle{ pSkinMeshComponent->GetBoneMatrixsUniform() }, boneMatrices.data(), static_cast<uint16_t>(boneMatrices.size()));
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