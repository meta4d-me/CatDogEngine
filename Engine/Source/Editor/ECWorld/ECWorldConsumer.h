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
class BlendShape;
class Camera;
class Light;
class Material;
class Mesh;
class Morph;
class Node;
class ParticleEmitter;
class SceneDatabase;
class Texture;
class VertexFormat;

}

namespace engine
{

class MaterialComponent;
class MaterialType;
class RenderContext;
class ResourceContext;
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

	void SetDefaultMaterialType(engine::MaterialType* pMaterialType) { m_pDefaultMaterialType = pMaterialType; }
	void SetSceneDatabaseIDs(uint32_t nodeID, uint32_t meshID);
	virtual void Execute(const cd::SceneDatabase* pSceneDatabase) override;

private:
	void AddCamera(engine::Entity entity, const cd::Camera& camera);
	void AddLight(engine::Entity entity, const cd::Light& light);
	void AddTransform(engine::Entity entity, const cd::Transform& transform);
	void AddStaticMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat);
	void AddSkinMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat, const cd::SceneDatabase* pSceneDatabase);
	void AddSkeleton(engine::Entity entity, const cd::SceneDatabase* pSceneDatabase);
	void AddAnimation(engine::Entity entity, const cd::Animation& animation, const cd::SceneDatabase* pSceneDatabase);
	void AddMaterial(engine::Entity entity, const cd::Material* pMaterial, engine::MaterialType* pMaterialType, const cd::SceneDatabase* pSceneDatabase);
	void AddBlendShape(engine::Entity entity, const cd::Mesh* pMesh, const cd::BlendShape& blendShape, const cd::SceneDatabase* pSceneDatabase);
	void AddParticleEmitter(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat, const cd::ParticleEmitter& emitter);
private:
	engine::MaterialType* m_pDefaultMaterialType = nullptr;
	engine::SceneWorld* m_pSceneWorld = nullptr;
	engine::RenderContext* m_pRenderContext = nullptr;
	engine::ResourceContext* m_pResourceContext = nullptr;

	uint32_t m_nodeMinID;
	uint32_t m_meshMinID;
};

}