#include "TerrainRenderer.h"

#include "Core/StringCrc.h"
#include "Framework/Processor.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Producers/CDProducer/CDProducer.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

#include <bgfx/bgfx.h>
#include <bimg/decode.h>
#include <bx/allocator.h>

#include <fstream>
#include <optional>

using namespace cd;

namespace
{
constexpr const char* kUniformSectorOrigin = "u_SectorOrigin";
//constexpr engine::StringCrc kUniformSectorOriginCrc(kUniformSectorOrigin);
constexpr const char* kUniformSectorDimension = "u_SectorDimension";
//constexpr engine::StringCrc kUniformSectorDimensionCrc(kUniformSectorDimension);

bx::AllocatorI* GetResourceAllocator()
{
	static bx::DefaultAllocator s_allocator;
	return &s_allocator;
}

std::vector<std::byte> LoadFile(const char* pFilePath)
{
	std::vector<std::byte> fileData;

	std::ifstream fin(pFilePath, std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		return fileData;
	}

	fin.seekg(0L, std::ios::end);
	size_t fileSize = fin.tellg();
	fin.seekg(0L, std::ios::beg);
	fileData.resize(fileSize);
	fin.read(reinterpret_cast<char*>(fileData.data()), fileSize);
	fin.close();

	return fileData;
}

}

namespace engine
{

void TerrainRenderer::Init()
{
	bgfx::setViewName(GetViewID(), "TerrainRenderer");
	m_updateUniforms = true;

	m_dirtTexture = CreateTerrainTexture("terrain/dirty_baseColor", 0);
	// TEMP CODE TODO move this to terrain editor
	m_redChannelTexture = CreateTerrainTexture("terrain/dirty_baseColor", 3);
	m_greenChannelTexture = CreateTerrainTexture("terrain/rockyGrass_baseColor", 4);
	m_blueChannelTexture = CreateTerrainTexture("terrain/gravel_baseColor", 5);
	m_alphaChannelTexture = CreateTerrainTexture("terrain/snowyRock_baseColor", 6);

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

		if (m_entityToRenderInfo.find(entity) == m_entityToRenderInfo.cend())
		{
			m_updateUniforms = true;
			UpdateUniforms();
		}

		// Check cull dist
		const Entity& cameraEntity = m_pCurrentSceneWorld->GetMainCameraEntity();
		const CameraComponent* cameraComponent = m_pCurrentSceneWorld->GetCameraComponent(cameraEntity);
		const float dx = cameraComponent->GetEye().x() - m_entityToRenderInfo[entity].m_origin[0];
		const float dy = cameraComponent->GetEye().y() - m_entityToRenderInfo[entity].m_origin[1];
		const float dz = cameraComponent->GetEye().z() - m_entityToRenderInfo[entity].m_origin[2];
		if (m_cullDistanceSquared <= (dx * dx + dy * dy + dz * dz)) {
			// skip
			continue;
		}

		const MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		const StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{pMeshComponent->GetVertexBuffer()});
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{pMeshComponent->GetIndexBuffer()});

		bgfx::setTexture(m_dirtTexture.slot, bgfx::UniformHandle{m_dirtTexture.samplerHandle}, bgfx::TextureHandle{m_dirtTexture.textureHandle});
		if (m_redChannelTexture.textureHandle != bgfx::kInvalidHandle && m_redChannelTexture.samplerHandle != bgfx::kInvalidHandle)
		{
			bgfx::setTexture(m_redChannelTexture.slot, bgfx::UniformHandle{m_redChannelTexture.samplerHandle}, bgfx::TextureHandle{m_redChannelTexture.textureHandle});
		}
		if (m_greenChannelTexture.textureHandle != bgfx::kInvalidHandle && m_greenChannelTexture.samplerHandle != bgfx::kInvalidHandle)
		{
			bgfx::setTexture(m_greenChannelTexture.slot, bgfx::UniformHandle{m_greenChannelTexture.samplerHandle}, bgfx::TextureHandle{m_greenChannelTexture.textureHandle});
		}
		if (m_blueChannelTexture.textureHandle != bgfx::kInvalidHandle && m_blueChannelTexture.samplerHandle != bgfx::kInvalidHandle)
		{
			bgfx::setTexture(m_blueChannelTexture.slot, bgfx::UniformHandle{m_blueChannelTexture.samplerHandle}, bgfx::TextureHandle{m_blueChannelTexture.textureHandle});
		}
		if (m_alphaChannelTexture.textureHandle != bgfx::kInvalidHandle && m_alphaChannelTexture.samplerHandle != bgfx::kInvalidHandle)
		{
			bgfx::setTexture(m_alphaChannelTexture.slot, bgfx::UniformHandle{m_alphaChannelTexture.samplerHandle}, bgfx::TextureHandle{m_alphaChannelTexture.textureHandle});
		}

		for (const auto& [textureType, textureInfo] : pMaterialComponent->GetTextureResources())
		{
			// TODO optimize by using textureInfo instead of GetTextureInfo
			if (const MaterialComponent::TextureInfo* pTextureInfo = pMaterialComponent->GetTextureInfo(textureType))
			{
				bgfx::setTexture(pTextureInfo->slot, bgfx::UniformHandle{pTextureInfo->samplerHandle}, bgfx::TextureHandle{pTextureInfo->textureHandle});
			}
		}

		const TerrainRenderInfo& meshRenderInfo = m_entityToRenderInfo[entity];
		bgfx::setUniform(u_terrainOrigin, static_cast<const void*>(meshRenderInfo.m_origin));
		bgfx::setUniform(u_terrainDimension, static_cast<const void*>(meshRenderInfo.m_dimension));

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		bgfx::submit(GetViewID(), bgfx::ProgramHandle{pMaterialComponent->GetShadingProgram()});
	}
}

void TerrainRenderer::SetAndLoadAlphaMapTexture(const cdtools::AlphaMapChannel channel, const std::string& textureName)
{
	switch (channel)
	{
	case cdtools::AlphaMapChannel::Red:
		m_redChannelTexture = CreateTerrainTexture(textureName.c_str(), 3);
		break;
	case cdtools::AlphaMapChannel::Green:
		m_greenChannelTexture = CreateTerrainTexture(textureName.c_str(), 4);
		break;
	case cdtools::AlphaMapChannel::Blue:
		m_blueChannelTexture = CreateTerrainTexture(textureName.c_str(), 5);
		break;
	case cdtools::AlphaMapChannel::Alpha:
		m_alphaChannelTexture = CreateTerrainTexture(textureName.c_str(), 6);
		break;
	default:
		assert(false);
	}
}

TerrainRenderer::TerrainTexture TerrainRenderer::CreateTerrainTexture(const char* textureFileName, uint8_t slot) 
{
	TerrainTexture outTexture;
	outTexture.slot = slot;
	outTexture.samplerHandle = bgfx::kInvalidHandle;
	outTexture.textureHandle = bgfx::kInvalidHandle;
	outTexture.format = cd::TextureFormat::Count;

	const std::string textureFilePath = engine::Path::GetTerrainTextureOutputFilePath(textureFileName, ".dds");
	// Load the texture file
	std::vector<std::byte> textureFileBlob = LoadFile(textureFilePath.c_str());
	assert(!textureFileBlob.empty());
	// Decode the texture
	bimg::ImageContainer* pImageContainer = bimg::imageParse(GetResourceAllocator(), textureFileBlob.data(), static_cast<uint32_t>(textureFileBlob.size()));
	const bgfx::Memory* pMemory = bgfx::makeRef(pImageContainer->m_data, pImageContainer->m_size);
	outTexture.format = static_cast<cd::TextureFormat>(pImageContainer->m_format);
	const uint64_t textureFlag = (BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_SRGB);
	outTexture.textureHandle = bgfx::createTexture2D(
		pImageContainer->m_width, 
		pImageContainer->m_height, 
		pImageContainer->m_numMips, 
		pImageContainer->m_numLayers, 
		static_cast<bgfx::TextureFormat::Enum>(pImageContainer->m_format),
		textureFlag, 
		pMemory
	).idx;

	std::string samplerUniformName = "s_terrainTextureSampler";
	samplerUniformName += std::to_string(slot);
	outTexture.samplerHandle = bgfx::createUniform(samplerUniformName.c_str(), bgfx::UniformType::Sampler).idx;
	assert(outTexture.textureHandle != bgfx::kInvalidHandle);
	assert(outTexture.samplerHandle != bgfx::kInvalidHandle);
	
	return outTexture;
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
			const MaterialComponent::TextureInfo* pElevationTexture = pMaterialComponent->GetTextureInfo(MaterialTextureType::Elevation);
			assert(pElevationTexture);
			renderInfo.m_dimension[0] = static_cast<float>(pElevationTexture->width);
			renderInfo.m_dimension[1] = static_cast<float>(pElevationTexture->height);
			renderInfo.m_dimension[2] = 0.0f;
			renderInfo.m_dimension[3] = 0.0f;
		}

		m_updateUniforms = false;
	}
}

}