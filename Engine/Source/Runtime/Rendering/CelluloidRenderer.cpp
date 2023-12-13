#include "CelluloidRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Scene/Texture.h"

namespace engine
{

	void CelluloidRenderer::Init()
	{
		constexpr StringCrc programCrc = StringCrc("CelluloidProgram");
		GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_celluloid", "fs_celluloid" });

		bgfx::setViewName(GetViewID(), "CelluloidRenderer");
	}

	void CelluloidRenderer::Warmup()
	{
		GetRenderContext()->UploadShaderProgram("CelluloidProgram");
	}

	void CelluloidRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
	{
		UpdateViewRenderTarget();
		bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
	}

	void CelluloidRenderer::Render(float deltaTime)
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

			if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
			{
				pTransformComponent->Build();
				bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
			}

			StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
			if (!pMeshComponent)
			{
				continue;
			}
			UpdateStaticMeshComponent(pMeshComponent);

			constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW;
			bgfx::setState(state);

			GetRenderContext()->Submit(GetViewID(), "CelluloidProgram");
		}
	}

}