#pragma once

#include "Base/Template.h"
#include "ECWorld/Entity.h"
#include "Framework/IConsumer.h"
#include "Scene/ObjectID.h"

#include <map>
#include <string>

namespace cd
{
class Material;
class Mesh;
class SceneDatabase;
class Texture;
class VertexFormat;
}

namespace engine
{
class MaterialType;
class RenderContext;
class SceneWorld;
class World;
}

namespace editor
{

class ECTerrainConsumer final : public cdtools::IConsumer
{
public:
	ECTerrainConsumer() = delete;
	explicit ECTerrainConsumer(engine::SceneWorld* pSceneWorld, engine::RenderContext* pRenderContext);
	ECTerrainConsumer(const ECTerrainConsumer&) = delete;
	ECTerrainConsumer& operator=(const ECTerrainConsumer&) = delete;
	ECTerrainConsumer(ECTerrainConsumer&&) = delete;
	ECTerrainConsumer& operator=(ECTerrainConsumer&&) = delete;
	virtual ~ECTerrainConsumer() = default;

	virtual void Execute(const cd::SceneDatabase* pSceneDatabase) override;

	void Clear();

private:
	void AddStaticMesh(engine::Entity entity, const cd::Mesh* mesh, const cd::VertexFormat& vertexFormat);
	void AddMaterial(engine::Entity entity, const cd::Material* pMaterial, engine::MaterialType* pMaterialType, const cd::SceneDatabase* pSceneDatabase);

	std::string GetShaderOutputFilePath(const char* pInputFilePath, const char* pAppendFileName = nullptr);
	std::string GetTextureOutputFilePath(const char* pInputFilePath);

	engine::RenderContext* m_pRenderContext;
	engine::SceneWorld* m_pSceneWorld;
	std::map<cd::MeshID::ValueType, engine::Entity> m_meshToEntity;
};

}
