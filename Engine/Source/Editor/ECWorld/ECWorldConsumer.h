#pragma once

#include "Base/Template.h"
#include "ECWorld/Entity.h"
#include "Framework/IConsumer.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/ObjectID.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

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

class ECWorldConsumer final : public cdtools::IConsumer
{
public:
	ECWorldConsumer() = delete;
	explicit ECWorldConsumer(engine::World* pWorld, engine::MaterialType* pMaterialType, engine::RenderContext* pRenderContext);
	ECWorldConsumer(const ECWorldConsumer&) = delete;
	ECWorldConsumer& operator=(const ECWorldConsumer&) = delete;
	ECWorldConsumer(ECWorldConsumer&&) = delete;
	ECWorldConsumer& operator=(ECWorldConsumer&&) = delete;
	virtual ~ECWorldConsumer() = default;

	void SetSceneDatabaseIDs(uint32_t nodeID);
	virtual void Execute(const cd::SceneDatabase* pSceneDatabase) override;

private:
	void AddNode(engine::Entity entity, const cd::Node& node);
	void AddMesh(engine::Entity entity, const cd::Mesh& mesh);
	std::string GetShaderOutputFilePath(const char* pInputFilePath, const char* pAppendFileName = nullptr);
	std::string GetTextureOutputFilePath(const char* pInputFilePath);
	void AddMaterial(engine::Entity entity, const cd::Material& material, const cd::SceneDatabase* pSceneDatabase);

private:
	engine::RenderContext* m_pRenderContext;
	engine::World* m_pWorld;
	engine::MaterialType* m_pStandardMaterialType;

	uint32_t m_nodeMinID;
	std::map<cd::NodeID::ValueType, engine::Entity> m_mapTransformIDToEntities;
};

}