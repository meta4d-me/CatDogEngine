#include "ECTerrainConsumer.h"

#include "ECWorld/ComponentsStorage.hpp"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ECWorld/World.h"
#include "Material/MaterialType.h"
#include "Rendering/RenderContext.h"
#include "Scene/Material.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/Node.h"
#include "Scene/SceneDatabase.h"
#include "Scene/VertexFormat.h"
#include "Producers/TerrainProducer/TerrainProducer.h"

namespace editor
{

ECTerrainConsumer::ECTerrainConsumer(engine::World* pWorld, engine::MaterialType* pMaterialType, engine::RenderContext* pRenderContext)
{
}

void ECTerrainConsumer::Execute(const cd::SceneDatabase* pSceneDatabase)
{

}

}