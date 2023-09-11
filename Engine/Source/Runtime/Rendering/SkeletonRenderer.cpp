#include "SkeletonRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkinMeshComponent.h"
#include "RenderContext.h"
#include "Rendering/Utility/VertexLayoutUtility.h"

namespace engine
{

namespace details
{

void CalculateBoneTranslate(const cd::Bone& bone, const cd::Matrix4x4& changeMatrix, const cd::SceneDatabase* pSceneDatabase, engine::SkinMeshComponent* pSkinMeshComponent)
{
	const uint32_t parentBoneID = bone.GetParentID().Data();
	for (auto& boneChild : bone.GetChildIDs())
	{
		pSkinMeshComponent->SetBoneChangeMatrix(boneChild.Data(), changeMatrix);
		CalculateBoneTranslate(pSceneDatabase->GetBone(boneChild.Data()), changeMatrix, pSceneDatabase, pSkinMeshComponent);
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

void SkeletonRenderer::Render(float delataTime)
{
	const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
	for (Entity entity : m_pCurrentSceneWorld->GetAnimationEntities())
	{
		auto pSkinMeshComponent = m_pCurrentSceneWorld->GetSkinMeshComponent(entity);
		if (!pSkinMeshComponent)
		{
			continue;
		}

		if (pSkinMeshComponent->GetChangeBoneIndex() != engine::INVALID_ENTITY)
		{
			const uint32_t changeBoneIndex = pSkinMeshComponent->GetChangeBoneIndex();
			const cd::Bone& bone = pSceneDatabase->GetBone(changeBoneIndex);
			const cd::Matrix4x4& changeBoneMatrix = pSkinMeshComponent->GetBoneChangeMatrix(changeBoneIndex);
			details::CalculateBoneTranslate(bone, changeBoneMatrix, pSceneDatabase, pSkinMeshComponent);
			pSkinMeshComponent->ResetChangeBoneIndex();
		}
		bgfx::setUniform(bgfx::UniformHandle{ pSkinMeshComponent->GetBoneMatrixsUniform() }, pSkinMeshComponent->GetBoneChangeMatrices().data(), static_cast<uint16_t>(pSkinMeshComponent->GetBoneChangeMatrices().size()));
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pSkinMeshComponent->GetBoneVBH() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pSkinMeshComponent->GetBoneIBH() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;

		bgfx::setState(state);
		constexpr StringCrc SkeletonProgram("SkeletonProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(SkeletonProgram));
	}
	
}

}