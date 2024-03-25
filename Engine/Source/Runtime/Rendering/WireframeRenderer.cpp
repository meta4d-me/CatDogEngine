#include "WireframeRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/MeshResource.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Scene/Texture.h"

#undef EDITOR_MODE

namespace engine
{

void WireframeRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("WireframeLineProgram", "vs_wireframe_line", "fs_wireframe_line"));

	bgfx::setViewName(GetViewID(), "WireframeRenderer");
}

void WireframeRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void WireframeRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

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

		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		const MeshResource* pMeshResource = pMeshComponent->GetMeshResource();
		if (ResourceStatus::Ready != pMeshResource->GetStatus() &&
			ResourceStatus::Optimized != pMeshResource->GetStatus())
		{
			continue;
		}

		if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LEQUAL |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;
		bgfx::setState(state);

		constexpr StringCrc programHandleIndex{ "ImGuiProgram" };
		SubmitStaticMeshDrawCall(pMeshComponent, GetViewID(), programHandleIndex);
	}
}

}