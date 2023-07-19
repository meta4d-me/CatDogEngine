#pragma once

#include "ECWorld/SceneWorld.h"
#include "Producers/TerrainProducer/AlphaMapTypes.h"
#include "Renderer.h"

#include <bgfx/bgfx.h>

#include <unordered_map>

namespace engine
{

class SceneWorld;

class TerrainRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }
	void SetCullDistance(uint32_t dist) { m_cullDistanceSquared = dist * dist; }
	void SetAndLoadAlphaMapTexture(const cdtools::AlphaMapChannel channel, const std::string& textureName);

private:
	struct TerrainRenderInfo
	{
		TerrainRenderInfo()
		{
			memset(m_origin, 0, sizeof(float) * 4);
			memset(m_dimension, 0, sizeof(float) * 4);
		}
		float m_origin[4];
		float m_dimension[4];
	};

	struct TerrainTexture
	{
		uint8_t slot;
		uint16_t samplerHandle = bgfx::kInvalidHandle;
		uint16_t textureHandle = bgfx::kInvalidHandle;
		cd::TextureFormat format;
	};

	TerrainTexture CreateTerrainTexture(const char* textureFileName, uint8_t slot);

	bool IsTerrainMesh(Entity) const;

	void UpdateUniforms();


	bool m_updateUniforms = true;
	SceneWorld* m_pCurrentSceneWorld = nullptr;
	std::unordered_map<Entity, TerrainRenderInfo> m_entityToRenderInfo;
	uint32_t m_cullDistanceSquared = 40000;

	// Textures
	TerrainTexture m_dirtTexture;
	TerrainTexture m_redChannelTexture;
	TerrainTexture m_greenChannelTexture;
	TerrainTexture m_blueChannelTexture;
	TerrainTexture m_alphaChannelTexture;
	// Uniforms
	bgfx::UniformHandle u_terrainOrigin;	// bottom left corner in world coord; vec2
	bgfx::UniformHandle u_terrainDimension;	// width and depth of the terrain; vec2
};

}