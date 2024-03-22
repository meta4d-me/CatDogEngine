#include "OutLineRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Scene/Texture.h"

namespace engine
{

namespace
{

constexpr const char* outLineColor = "u_outLineColor";
constexpr const char* outLineSize  = "u_outLineSize";

}

void OutLineRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("OutLineProgram", "vs_outline", "fs_outline"));

	GetRenderContext()->CreateUniform(outLineColor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(outLineSize, bgfx::UniformType::Vec4, 1);

	bgfx::setViewName(GetViewID(), "OutLineRenderer");
}

void OutLineRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void OutLineRenderer::Render(float deltaTime)
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
		if (!pMaterialComponent->GetToonParameters().isOpenOutLine)
		{
			continue;
		}
		if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}

		constexpr StringCrc outLineColorCrc(outLineColor);
		GetRenderContext()->FillUniform(outLineColorCrc, pMaterialComponent->GetToonParameters().outLineColor.begin());

		constexpr StringCrc outLineSizeCrc(outLineSize);
		GetRenderContext()->FillUniform(outLineSizeCrc, &pMaterialComponent->GetToonParameters().outLineSize);

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW;
		bgfx::setState(state);

		constexpr StringCrc programHandleIndex{ "OutLineProgram" };
		SubmitStaticMeshDrawCall(pMeshComponent, GetViewID(), programHandleIndex);
	}
}

}