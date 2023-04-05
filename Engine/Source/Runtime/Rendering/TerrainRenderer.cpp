#include "TerrainRenderer.h"

#include "Framework/Processor.h"
#include "Log/Log.h"
#include "Producers/CDProducer/CDProducer.h"
#include "RenderContext.h"
#include "Scene/Texture.h"
#include "Core/StringCrc.h"

#include <bgfx/bgfx.h>

#include <optional>

using namespace cd;

namespace
{
constexpr const char* kUniformSectorOrigin = "u_SectorOrigin";
constexpr engine::StringCrc kUniformSectorOriginCrc(kUniformSectorOrigin);
constexpr const char* kUniformSectorDimension = "u_SectorDimension";
constexpr engine::StringCrc kUniformSectorDimensionCrc(kUniformSectorDimension);
}

namespace engine
{

void TerrainRenderer::Init()
{
	bgfx::setViewName(GetViewID(), "TerrainRenderer");
	m_updateUniforms = true;

	u_terrainOrigin = m_pRenderContext->CreateUniform(kUniformSectorOrigin, bgfx::UniformType::Enum::Vec4, 1);
	u_terrainDimension = m_pRenderContext->CreateUniform(kUniformSectorDimension, bgfx::UniformType::Vec4, 1);
}

void TerrainRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);

	UpdateUniforms();
}

void TerrainRenderer::Render(float deltaTime)
{
	for (const Entity& entity : m_pCurrentSceneWorld->GetMaterialEntities())
	{
		if (!IsTerrainMesh(entity))
		{
			continue;
		}
		const MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		const StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle(pMeshComponent->GetVertexBuffer()));
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle(pMeshComponent->GetIndexBuffer()));

		for (const auto& [textureType, textureInfo] : pMaterialComponent->GetTextureResources())
		{
			std::optional<const MaterialComponent::TextureInfo> optTextureInfo = pMaterialComponent->GetTextureInfo(textureType);
			if (optTextureInfo.has_value())
			{
				const MaterialComponent::TextureInfo& textureInfo = optTextureInfo.value();
				bgfx::setTexture(textureInfo.slot, bgfx::UniformHandle(textureInfo.samplerHandle), bgfx::TextureHandle(textureInfo.textureHandle));
			}
		}
		
		if (m_entityToRenderInfo.find(entity) == m_entityToRenderInfo.cend())
		{
			m_updateUniforms = true;
			UpdateUniforms();
		}
		const TerrainRenderInfo& meshRenderInfo = m_entityToRenderInfo[entity];
		bgfx::setUniform(u_terrainOrigin, static_cast<const void*>(meshRenderInfo.m_origin));
		bgfx::setUniform(u_terrainDimension, static_cast<const void*>(meshRenderInfo.m_dimension));

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		bgfx::submit(GetViewID(), bgfx::ProgramHandle(pMaterialComponent->GetShadingProgram()));
	}
}

bool TerrainRenderer::IsTerrainMesh(Entity entity) const
{
	const MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
	if (!pMaterialComponent ||
		pMaterialComponent->GetMaterialType() != m_pCurrentSceneWorld->GetTerrainMaterialType())
	{
		return false;
	}
	const StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
	if (!pMeshComponent)
	{
		CD_ENGINE_WARN("Entity: %u has terrain material but is missing mesh component!", entity);
		return false;
	}
	return true;
}

void TerrainRenderer::UpdateUniforms()
{
	if (m_updateUniforms)
	{
		for (const Entity& entity : m_pCurrentSceneWorld->GetMaterialEntities())
		{
			if (!IsTerrainMesh(entity))
			{
				continue;
			}
			if (m_entityToRenderInfo.find(entity) == m_entityToRenderInfo.cend())
			{
				m_entityToRenderInfo[entity] = TerrainRenderInfo();
			}
			TerrainRenderInfo& renderInfo = m_entityToRenderInfo[entity];
			const StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
			const Mesh* terrainMesh = pMeshComponent->GetMeshData();
			if (!terrainMesh)
			{
				CD_ENGINE_WARN("Entity: %u has null mesh data!", entity);
				continue;
			}
			// Convention is determined by TerrainProducer that the origin is always the first vertex
			const std::vector<Point>& vertices = terrainMesh->GetVertexPositions();
			const Point& origin = vertices[0];
			renderInfo.m_origin[0] = origin.x();
			renderInfo.m_origin[1] = origin.y();
			renderInfo.m_origin[2] = origin.z();
			renderInfo.m_origin[3] = 0.0f;

			const MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
			std::optional<const MaterialComponent::TextureInfo> elevationTexture = pMaterialComponent->GetTextureInfo(MaterialTextureType::Roughness);
			assert(elevationTexture.has_value());
			renderInfo.m_dimension[0] = static_cast<float>(elevationTexture->width);
			renderInfo.m_dimension[1] = static_cast<float>(elevationTexture->height);
			renderInfo.m_dimension[2] = 0.0f;
			renderInfo.m_dimension[3] = 0.0f;
		}

		m_updateUniforms = false;
	}
}

}