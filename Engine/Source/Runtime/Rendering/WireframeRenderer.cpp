#include "WireframeRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/ShaderVariantCollections.h"
#include "Scene/Texture.h"

namespace engine
{

void WireframeRenderer::Init()
{
	GetShaderVariantCollections()->RegisterNonUberShader("WireframeLineProgram", { "vs_wireframe_line", "fs_wireframe_line" });

	bgfx::setViewName(GetViewID(), "WireframeRenderer");
}

void WireframeRenderer::Submit()
{
	GetRenderContext()->CreateProgram("WireframeLineProgram", "vs_wireframe_line.bin", "fs_wireframe_line.bin");
}

void WireframeRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void WireframeRenderer::Render(float deltaTime)
{
	for (Entity entity : m_pCurrentSceneWorld->GetStaticMeshEntities())
	{
		if (!m_enableGlobalWireframe && m_pCurrentSceneWorld->GetSelectedEntity() != entity)
		{
			continue;
		}

		if (m_pCurrentSceneWorld->GetSkyEntity() == entity)
		{
			continue;
		}

		TerrainComponent* pTerrainComponent = m_pCurrentSceneWorld->GetTerrainComponent(entity);
		if (pTerrainComponent)
		{
			continue;
		}

		if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		StaticMeshComponent* pStaticMesh = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pStaticMesh->GetVertexBuffer() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pStaticMesh->GetWireframeIndexBuffer() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LEQUAL |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;
		bgfx::setState(state);

		constexpr StringCrc whiteModelProgram("WireframeLineProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(whiteModelProgram));
	}
}

}