#include "DebugRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

namespace engine
{

void DebugRenderer::Init()
{
	auto* pShaderVariantCollectionsComponent = m_pCurrentSceneWorld->GetShaderVariantCollectionsComponent(m_pCurrentSceneWorld->GetShaderVariantCollectionEntity());

	pShaderVariantCollectionsComponent->AddShader("vs_debug");
	pShaderVariantCollectionsComponent->AddShader("fs_debug");

	bgfx::setViewName(GetViewID(), "DebugRenderer");
}

void DebugRenderer::LoadShaders()
{
	GetRenderContext()->CreateProgram("DebugProgram", "vs_debug.bin", "fs_debug.bin");
}

void DebugRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void DebugRenderer::Render(float deltaTime)
{
	for (Entity entity : m_pCurrentSceneWorld->GetStaticMeshEntities())
	{
		if (m_pCurrentSceneWorld->GetSkyEntity() == entity)
		{
			continue;
		}

		TerrainComponent* pTerrainComponent = m_pCurrentSceneWorld->GetTerrainComponent(entity);
		if (pTerrainComponent)
		{
			continue;
		}

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

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pMeshComponent->GetVertexBuffer() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pMeshComponent->GetIndexBuffer() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		bgfx::setState(state);

		constexpr StringCrc debugProgram("DebugProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(debugProgram));
	}
}

}