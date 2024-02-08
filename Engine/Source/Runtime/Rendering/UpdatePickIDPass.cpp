#include "UpdatePickIDPass.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Scene/Texture.h"

namespace engine
{

void UpdatePickIDPass::Init()
{
	constexpr StringCrc programCrc = StringCrc("WhiteModelProgram");
	GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_whiteModel", "fs_whiteModel" });

	bgfx::setViewName(GetViewID(), "UpdatePickIDPass");
}

void UpdatePickIDPass::Warmup()
{
	GetRenderContext()->UploadShaderProgram("WhiteModelProgram");
}

void UpdatePickIDPass::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void UpdatePickIDPass::Render(float deltaTime)
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
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}

		// No mesh attached?
		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}
		UpdateStaticMeshComponent(pMeshComponent);

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		bgfx::setState(state);

		GetRenderContext()->Submit(GetViewID(), "WhiteModelProgram");
	}
}

}