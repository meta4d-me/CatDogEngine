#pragma once

#include "Base/Template.h"
#include "Framework/IConsumer.h"

namespace cd
{

	class Material;
	class Mesh;
	class Node;
	class SceneDatabase;
	class Texture;

}

namespace engine
{

	class MaterialComponent;
	class MaterialType;
	class RenderContext;
	class World;

}

namespace editor
{

class ECTerrainConsumer final : public cdtools::IConsumer
{
public:
	ECTerrainConsumer() = delete;
	explicit ECTerrainConsumer(engine::World* pWorld, engine::MaterialType* pMaterialType, engine::RenderContext* pRenderContext);
	ECTerrainConsumer(const ECTerrainConsumer&) = delete;
	ECTerrainConsumer& operator=(const ECTerrainConsumer&) = delete;
	ECTerrainConsumer(ECTerrainConsumer&&) = delete;
	ECTerrainConsumer& operator=(ECTerrainConsumer&&) = delete;
	virtual ~ECTerrainConsumer() = default;

	virtual void Execute(const cd::SceneDatabase* pSceneDatabase) override;
};

}
