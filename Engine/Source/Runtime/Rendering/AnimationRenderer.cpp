#include "AnimationRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Display/Camera.h"
#include "RenderContext.h"
#include "Scene/Animation.h"
#include "Scene/Texture.h"

#include <format>

namespace engine
{

namespace detail
{

void CalculateBoneTransform(std::vector<cd::Matrix4x4>& boneMatrices, const cd::SceneDatabase* pSceneDatabase,
	float animationTime, const cd::Bone* pBone, const cd::Matrix4x4& parentTransform)
{
	auto CalculateInterpolatedTranslation = [&animationTime](const cd::Track* pTrack) -> cd::Matrix4x4
	{
		if (1U == pTrack->GetTranslationKeyCount())
		{
			const auto& firstKey = pTrack->GetTranslationKeys()[0];
			cd::Transform identity = cd::Transform::Identity();
			identity.SetTranslation(firstKey.GetValue());
			return identity.GetMatrix();
		}

		//for (uint32_t keyIndex = 0U; keyIndex < pTrack->GetTranslationKeyCount() - 1; ++keyIndex)
		//{
		//	const auto& nextKey = pTrack->GetTranslationKeys()[keyIndex + 1];
		//	if (animationTime < nextKey.GetTime())
		//	{
		//		const auto& currentKey = pTrack->GetTranslationKeys()[keyIndex];
		//		float keyFrameDeltaTime = nextKey.GetTime() - currentKey.GetTime();
		//		float keyFrameRate = (animationTime - currentKey.GetTime()) / keyFrameDeltaTime;
		//		assert(keyFrameRate >= 0.0f && keyFrameRate <= 1.0f);
		//		return cd::Vec3f::Lerp(currentKey.GetValue(), nextKey.GetValue(), keyFrameRate);
		//	}
		//}

		return cd::Matrix4x4::Identity();
	};

	auto CalculateInterpolatedRotation = [&animationTime](const cd::Track* pTrack) -> cd::Matrix4x4
	{
		if (1U == pTrack->GetRotationKeyCount())
		{
			const auto& firstKey = pTrack->GetRotationKeys()[0];
			cd::Transform identity = cd::Transform::Identity();
			identity.SetRotation(firstKey.GetValue());
			return identity.GetMatrix();
		}

		return cd::Matrix4x4::Identity();
	};

	auto CalculateInterpolatedScale = [&animationTime](const cd::Track* pTrack) -> cd::Matrix4x4
	{
		if (1U == pTrack->GetScaleKeyCount())
		{
			const auto& firstKey = pTrack->GetScaleKeys()[0];
			cd::Transform identity = cd::Transform::Identity();
			identity.SetScale(firstKey.GetValue());
			return identity.GetMatrix();
		}

		return cd::Matrix4x4::Identity();
	};

	cd::Matrix4x4 boneTransform = cd::Matrix4x4::Identity();
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName(pBone->GetName()))
	{
		assert(pTrack->GetTranslationKeyCount() > 0);
		assert(pTrack->GetRotationKeyCount() > 0);
		assert(pTrack->GetScaleKeyCount() > 0);
		cd::Matrix4x4 translation = CalculateInterpolatedTranslation(pTrack);
		cd::Matrix4x4 roatation = CalculateInterpolatedRotation(pTrack);
		cd::Matrix4x4 scale = CalculateInterpolatedScale(pTrack);
		boneTransform = translation * roatation * scale;
	}

	cd::Matrix4x4 globalTransform = parentTransform * boneTransform * pBone->GetTransform().GetMatrix();
	boneMatrices[pBone->GetID().Data()] = globalTransform;

	for (cd::BoneID boneID : pBone->GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		CalculateBoneTransform(boneMatrices, pSceneDatabase, animationTime, &childBone, globalTransform);
	}
};

}

void AnimationRenderer::Init()
{
#ifdef VISUALIZE_BONE_WEIGHTS
	m_pRenderContext->CreateUniform("u_debugBoneIndex", bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateProgram("AnimationProgram", "vs_visualize_bone_weight.bin", "fs_visualize_bone_weight.bin");
#else
	m_pRenderContext->CreateProgram("AnimationProgram", "vs_animation.bin", "fs_animation.bin");
#endif

	bgfx::setViewName(GetViewID(), "AnimationRenderer");
}

void AnimationRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());
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

	constexpr StringCrc boneIndexUniform("u_debugBoneIndex");
	bgfx::setUniform(m_pRenderContext->GetUniform(boneIndexUniform), selectedBoneIndex, 1);
#endif

	static float animationRunningTime = 0.0f;
	animationRunningTime += deltaTime;

	const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
	for (Entity entity : m_pCurrentSceneWorld->GetAnimationEntities())
	{
		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		AnimationComponent* pAnimationComponent = m_pCurrentSceneWorld->GetAnimationComponent(entity);

		const cd::Animation* pAnimation = pAnimationComponent->GetAnimationData();
		float ticksPerSecond = pAnimation->GetTicksPerSecnod();
		assert(ticksPerSecond > 1.0f);
		float animationTime = std::fmodf(animationRunningTime * ticksPerSecond, pAnimation->GetDuration());

		std::vector<cd::Matrix4x4> boneMatrices;
		boneMatrices.reserve(pSceneDatabase->GetBoneCount());
		for (const auto& bone : pSceneDatabase->GetBones())
		{
			boneMatrices.push_back(cd::Matrix4x4::Identity());
		}
		detail::CalculateBoneTransform(boneMatrices, pSceneDatabase, animationTime, &pSceneDatabase->GetBone(0), cd::Matrix4x4::Identity());

		bgfx::setUniform(bgfx::UniformHandle(pAnimationComponent->GetBoneMatrixsUniform()), boneMatrices.data(), static_cast<uint16_t>(boneMatrices.size()));

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle(pMeshComponent->GetVertexBuffer()));
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle(pMeshComponent->GetIndexBuffer()));

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		constexpr StringCrc animationProgram("AnimationProgram");
		bgfx::submit(GetViewID(), m_pRenderContext->GetProgram(animationProgram));
	}
}

}