#pragma once

#include "ECWorld/SceneWorld.h"
#include "MeshRenderData.h"
#include "Renderer.h"

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

	bool IsTerrainMesh(Entity) const;

	void UpdateUniforms();

	bool m_updateUniforms = true;
	SceneWorld* m_pCurrentSceneWorld = nullptr;
	std::unordered_map<Entity, TerrainRenderInfo> m_entityToRenderInfo;

	bgfx::UniformHandle u_terrainOrigin;	// bottom left corner in world coord; vec2
	bgfx::UniformHandle u_terrainDimension;	// width and depth of the terrain; vec2
};

}