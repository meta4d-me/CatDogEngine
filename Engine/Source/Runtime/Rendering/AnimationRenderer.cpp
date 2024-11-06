#include "AnimationRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/MeshResource.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Scene/Texture.h"

#include <cmath>

//#define VISUALIZE_BONE_WEIGHTS
namespace engine
{

namespace
{

constexpr const char* debugBoneIndex = "u_debugBoneIndex";

}

namespace details
{

float CustomFModf(float dividend, float divisor)
{
	if (divisor == 0.0f)
		return 0.0f;

	int quotient = static_cast<int>(dividend / divisor);
	float result = dividend - static_cast<float>(quotient) * divisor;

	// Handle potential precision issues near zero
	if (result == 0.0f && dividend != 0.0f)
		result = 0.0f;

	// Ensure the result has the same sign as the divisor
	if ((dividend < 0 && divisor > 0) || (dividend > 0 && divisor < 0))
		result = -result;

	return result;
}

void CalculateBoneTransform(std::vector<cd::Matrix4x4>& boneMatrices, const cd::SceneDatabase* pSceneDatabase,
	float animationTime, const cd::Bone& bone, const cd::Matrix4x4& parentBoneTransform, const cd::Matrix4x4& globalInverse)
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

	cd::Matrix4x4 boneLocalTransform = bone.GetTransform().GetMatrix();
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName(bone.GetName()))
	{
		boneLocalTransform = cd::Transform(CalculateInterpolatedTranslation(pTrack),
									  CalculateInterpolatedRotation(pTrack),
									  CalculateInterpolatedScale(pTrack)).GetMatrix();
	}

	cd::Matrix4x4 globalTransform = parentBoneTransform * boneLocalTransform;
	boneMatrices[bone.GetID().Data()] = globalInverse * globalTransform * bone.GetOffset();

	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		CalculateBoneTransform(boneMatrices, pSceneDatabase, animationTime, childBone, globalTransform, globalInverse);
	}
};

}

void AnimationRenderer::Init()
{
	bgfx::setViewName(GetViewID(), "AnimationRenderer");

#ifdef VISUALIZE_BONE_WEIGHTS
	GetRenderContext()->CreateUniform(debugBoneIndex, bgfx::UniformType::Vec4, 1);
#endif
}

void AnimationRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void AnimationRenderer::Render(float deltaTime)
{
#ifdef VISUALIZE_BONE_WEIGHTS
	constexpr float changeTime = 0.2f;
	static float passedTime = 0.0f;
	static float selectedBoneIndex[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	passedTime += deltaTime;
	if (passedTime > changeTime)
	{
		selectedBoneIndex[0] = selectedBoneIndex[0] + 1.0f;
		if (selectedBoneIndex[0] > 100.0f)
		{
			selectedBoneIndex[0] = 0.0f;
		}
		passedTime -= changeTime;
	}

	constexpr StringCrc boneIndexCrc(debugBoneIndex);
	GetRenderContext()->FillUniform(boneIndexCrc, selectedBoneIndex, 1);
#endif

	static float animationRunningTime = 0.0f;
	animationRunningTime += deltaTime;

	const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
	for (Entity entity : m_pCurrentSceneWorld->GetStaticMeshEntities())
	{
		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		if (!pMaterialComponent ||
			pMaterialComponent->GetMaterialType() != m_pCurrentSceneWorld->GetAnimationMaterialType())
		{
			continue;
		}

		const ShaderResource* pShaderResource = pMaterialComponent->GetShaderResource();
		if (ResourceStatus::Ready != pShaderResource->GetStatus() &&
			ResourceStatus::Optimized != pShaderResource->GetStatus())
		{
			continue;
		}

		TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity);
		bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		GetRenderContext()->Submit(GetViewID(), pShaderResource->GetHandle());
	}
}

}