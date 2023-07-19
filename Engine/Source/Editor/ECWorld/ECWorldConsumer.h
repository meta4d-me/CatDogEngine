#pragma once

#include "Base/Template.h"
#include "ECWorld/Entity.h"
#include "Framework/IConsumer.h"
#include "Material/ShaderSchema.h"
#include "Math/Transform.hpp"
#include "Scene/MaterialTextureType.h"
#include "Scene/ObjectID.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace cd
{

class Animation;
class Camera;
class Light;
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

enum class MeshAssetType : uint8_t
{
	Standard,
	DDGI,
};

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

	void SetSceneDatabaseIDs(uint32_t nodeID, uint32_t meshID);
	virtual void Execute(const cd::SceneDatabase* pSceneDatabase) override;

	void ActivateUberOption(cd::MaterialTextureType textureType);
	void DeactivateUberOption(cd::MaterialTextureType textureType);
	void ClearActiveUberOption();

	std::set<engine::Uber>& GetActiveUberOptions() { return m_activeUberOptions; }
	const std::set<engine::Uber>& GetActiveUberOptions() const { return m_activeUberOptions; }

	void ActivateDDGIService() { m_meshAssetType = MeshAssetType::DDGI; }

private:
	void AddCamera(engine::Entity entity, const cd::Camera& camera);
	void AddLight(engine::Entity entity, const cd::Light& light);
	void AddTransform(engine::Entity entity, const cd::Transform& transform);
	void AddStaticMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat);
	void AddSkinMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat);
	void AddAnimation(engine::Entity entity, const cd::Animation& animation, const cd::SceneDatabase* pSceneDatabase);
	void AddMaterial(engine::Entity entity, const cd::Material* pMaterial, engine::MaterialType* pMaterialType, const cd::SceneDatabase* pSceneDatabase);

private:
	engine::SceneWorld* m_pSceneWorld;

	uint32_t m_nodeMinID;
	uint32_t m_meshMinID;
	std::set<engine::Uber> m_activeUberOptions;

	MeshAssetType m_meshAssetType = MeshAssetType::Standard;
};

}