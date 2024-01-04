#include "OutLineRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Scene/Texture.h"

namespace engine
{

	void OutLineRenderer::Init()
	{
		constexpr StringCrc programCrc = StringCrc("OutLineProgram");
		GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_outline", "fs_outline" });

		bgfx::setViewName(GetViewID(), "OutLineRenderer");
	}

	void OutLineRenderer::Warmup()
	{
		GetRenderContext()->UploadShaderProgram("OutLineProgram");
	}

	void OutLineRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
	{
		UpdateViewRenderTarget();
		bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
	}

	void OutLineRenderer::Render(float deltaTime)
	{
		for (Entity entity : m_pCurrentSceneWorld->GetStaticMeshEntities())
		{
			StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);

			if (m_pCurrentSceneWorld->GetSkyEntity() == entity)
			{
				continue;
			}

			TerrainComponent* pTerrainComponent = m_pCurrentSceneWorld->GetTerrainComponent(entity);
			if (pTerrainComponent)
			{
				continue;
			}
			if (!pMeshComponent)
			{
				continue;
			}

			MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
			if (!pMaterialComponent->GetOutLine())
			{
				continue;
			}
			if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
			{
				pTransformComponent->Build();
				bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
			}
			UpdateStaticMeshComponent(pMeshComponent);
			constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW;
			bgfx::setState(state);

			GetRenderContext()->Submit(GetViewID(), "OutLineProgram");
		}
	}

}