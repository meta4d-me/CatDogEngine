#include "AnimationRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Display/Camera.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

#include <format>

namespace engine
{

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
#else
	constexpr float changeTime = 0.2f;
	static float passedTime = 0.0f;
	static float selectedBoneIndex[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	passedTime += deltaTime;
	if (passedTime > changeTime)
	{
		selectedBoneIndex[0] = selectedBoneIndex[0] + 1.0f;
		if (selectedBoneIndex[0] > 64.0f)
		{
			selectedBoneIndex[0] = 0.0f;
		}
		passedTime -= changeTime;
	}
#endif

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
		const cd::Track* pTrackData = pAnimationComponent->GetTrackData();
		for (cd::TrackID trackID : pAnimation->GetBoneTrackIDs())
		{
			const cd::Track* pTrack = pTrackData + trackID.Data();
			const cd::Bone* pBone = pSceneDatabase->GetBoneByName(pTrack->GetName());
		}
		std::vector<cd::Matrix4x4> boneMatrices;
		boneMatrices.reserve(pSceneDatabase->GetBoneCount());
		for (const auto& bone : pSceneDatabase->GetBones())
		{
			boneMatrices.push_back(cd::Matrix4x4::Identity());
		}

		{
			const auto& bone = pSceneDatabase->GetBone(static_cast<uint32_t>(selectedBoneIndex[0]));
			const char* pBoneName = bone.GetName();
			const cd::Transform& localToBone = bone.GetTransform();
			boneMatrices[bone.GetID().Data()] = localToBone.GetMatrix();
		}

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