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
class VertexFormat;

}

namespace engine
{

class MaterialComponent;
class MaterialType;
class RenderContext;
class SceneWorld;

}

namespace editor
{

class ECWorldConsumer final : public cdtools::IConsumer
{
public:
	ECWorldConsumer() = delete;
	explicit ECWorldConsumer(engine::SceneWorld* pSceneWorld, engine::RenderContext* pRenderContext);
	ECWorldConsumer(const ECWorldConsumer&) = delete;
	ECWorldConsumer& operator=(const ECWorldConsumer&) = delete;
	ECWorldConsumer(ECWorldConsumer&&) = delete;
	ECWorldConsumer& operator=(ECWorldConsumer&&) = delete;
	virtual ~ECWorldConsumer() = default;

	void SetSceneDatabaseIDs(uint32_t nodeID);
	virtual void Execute(const cd::SceneDatabase* pSceneDatabase) override;

private:
	void AddNode(engine::Entity entity, const cd::Node& node);
	void AddStaticMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat);
	void AddSkinMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat);
	void AddMaterial(engine::Entity entity, const cd::Material* pMaterial, engine::MaterialType* pMaterialType, const cd::SceneDatabase* pSceneDatabase);

	std::string GetShaderOutputFilePath(const char* pInputFilePath, const char* pAppendFileName = nullptr);
	std::string GetTextureOutputFilePath(const char* pInputFilePath);

private:
	engine::RenderContext* m_pRenderContext;
	engine::SceneWorld* m_pSceneWorld;

	uint32_t m_nodeMinID;
	std::map<cd::NodeID::ValueType, engine::Entity> m_mapTransformIDToEntities;
};

}