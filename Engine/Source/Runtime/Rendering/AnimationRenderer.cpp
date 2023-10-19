#include "AnimationRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "RenderContext.h"
#include "Scene/Texture.h"
#include "Log/Log.h"
#include <cmath>
//#define VISUALIZE_BONE_WEIGHTS
namespace engine
{

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
	boneMatrices[bone.GetID().Data()] = bone.GetOffset() * globalTransform;

	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		CalculateBoneTransform(boneMatrices, pSceneDatabase, animationTime, childBone, globalTransform, globalInverse);
	}
};

void CalculateVertexTransform(std::vector<cd::Matrix4x4>& vertexMatrices, const cd::SceneDatabase* pSceneDatabase,
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

	cd::Matrix4x4 vertexLocalTransform = cd::Matrix4x4::Identity();
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName(bone.GetName()))
	{
		vertexLocalTransform = cd::Transform(CalculateInterpolatedTranslation(pTrack),
			CalculateInterpolatedRotation(pTrack),
			CalculateInterpolatedScale(pTrack)).GetMatrix() * bone.GetOffset();
	}

	vertexMatrices[bone.GetID().Data()] = vertexLocalTransform;

	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		CalculateVertexTransform(vertexMatrices, pSceneDatabase, animationTime, childBone);
	}
}

}

void AnimationRenderer::Init()
{
#ifdef VISUALIZE_BONE_WEIGHTS
	GetRenderContext()->CreateUniform("u_debugBoneIndex", bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateProgram("AnimationProgram", "vs_visualize_bone_weight.bin", "fs_visualize_bone_weight.bin");
#else
	GetRenderContext()->CreateProgram("AnimationProgram", "vs_animation.bin", "fs_animation.bin");
#endif

	bgfx::setViewName(GetViewID(), "AnimationRenderer");
}

void AnimationRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void AnimationRenderer::Render(float deltaTime)
{
#ifdef VISUALIZE_BONE_WEIGHTS
	float boneIndex = static_cast<float>(m_pCurrentSceneWorld->GetSelectedBoneID().Data());
	float selectedBoneIndex[4] = { boneIndex, 0.0f, 0.0f, 0.0f };
	constexpr StringCrc boneIndexUniform("u_debugBoneIndex");
	bgfx::setUniform(GetRenderContext()->GetUniform(boneIndexUniform), selectedBoneIndex, 1);
#endif

	static float animationRunningTime = 0.0f;
	animationRunningTime += deltaTime / 20.0f;

	const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
	for (Entity entity : m_pCurrentSceneWorld->GetAnimationEntities())
	{
		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity);
		bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());

		AnimationComponent* pAnimationComponent = m_pCurrentSceneWorld->GetAnimationComponent(entity);

		const cd::Animation* pAnimation = pAnimationComponent->GetAnimationData();
		float ticksPerSecond = pAnimation->GetTicksPerSecnod();
		assert(ticksPerSecond > 1.0f);
		float animationTime = details::CustomFModf(animationRunningTime * ticksPerSecond, pAnimation->GetDuration());

		static std::vector<cd::Matrix4x4> boneMatrices;
		boneMatrices.clear();
		for (uint16_t boneIndex = 0; boneIndex < 128; ++boneIndex)
		{
			boneMatrices.push_back(cd::Matrix4x4::Identity());
		}

		static std::vector<cd::Matrix4x4> vertexMatrices;
		vertexMatrices.clear();
		for (uint16_t boneIndex = 0; boneIndex < 128; ++boneIndex)
		{
			vertexMatrices.push_back(cd::Matrix4x4::Identity());
		}

		const cd::Bone& rootBone = pSceneDatabase->GetBone(0);
		details::CalculateVertexTransform(vertexMatrices, pSceneDatabase, animationTime, rootBone);
		bgfx::setUniform(bgfx::UniformHandle{pAnimationComponent->GetVertexMatrixsUniform()}, vertexMatrices.data(), static_cast<uint16_t>(vertexMatrices.size()));
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{pMeshComponent->GetVertexBuffer()});
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{pMeshComponent->GetIndexBuffer()});

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		constexpr StringCrc animationProgram("AnimationProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(animationProgram));
	}
}

}