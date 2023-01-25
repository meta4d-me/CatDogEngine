#pragma once

#include "ECWorld/CameraComponent.h"
#include "ECWorld/HierarchyComponent.h"
#include "ECWorld/LightComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/World.h"
#include "ECWorld/SkyComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"

#include "Scene/SceneDatabase.h"

#include <memory>
#include <vector>

namespace engine
{

class CameraComponent;
class HierarchyComponent;
class LightComponent;
class MaterialComponent;
class NameComponent;
class SkyComponent;
class StaticMeshComponent;
class TransformComponent;

class SceneWorld
{
public:
	SceneWorld();
	SceneWorld(const SceneWorld&) = default;
	SceneWorld& operator=(const SceneWorld&) = default;
	SceneWorld(SceneWorld&&) = default;
	SceneWorld& operator=(SceneWorld&&) = default;
	~SceneWorld() = default;

	cd::SceneDatabase* GetSceneDatabase() { return m_pSceneDatabase.get(); }
	engine::World* GetWorld() { return m_pWorld.get(); }
	const engine::World* GetWorld() const { return m_pWorld.get(); }

	void SetSelectedEntity(engine::Entity entity) { m_selectedEntity = entity; }
	engine::Entity GetSelectedEntity() const { return m_selectedEntity; }

	// It can save performance on addressing the actual ComponentStorage.
	// TODO : write a help macro ? Though I hate macro...
	engine::CameraComponent* GetCameraComponent(engine::Entity entity) const { return m_pCameraStorage->GetComponent(entity); }
	engine::HierarchyComponent* GetHierarchyComponent(engine::Entity entity) const { return m_pHierarchyStorage->GetComponent(entity); }
	engine::LightComponent* GetLightComponent(engine::Entity entity) const { return m_pLightStorage->GetComponent(entity); }
	engine::MaterialComponent* GetMaterialComponent(engine::Entity entity) const { return m_pMaterialStorage->GetComponent(entity); }
	engine::NameComponent* GetNameComponent(engine::Entity entity) const { return m_pNameStorage->GetComponent(entity); }
	engine::SkyComponent* GetSkyComponent(engine::Entity entity) const { return m_pSkyStorage->GetComponent(entity); }
	engine::StaticMeshComponent* GetStaticMeshComponent(engine::Entity entity) const { return m_pStaticMeshStorage->GetComponent(entity); }
	engine::TransformComponent* GetTransformComponent(engine::Entity entity) const { return m_pTransformStorage->GetComponent(entity); }

	const std::vector<engine::Entity>& GetCameraEntities() const { return m_pCameraStorage->GetEntities(); }
	const std::vector<engine::Entity>& GetHierarchyEntities() const { return m_pHierarchyStorage->GetEntities(); }
	const std::vector<engine::Entity>& GetLightEntities() const { return m_pLightStorage->GetEntities(); }
	const std::vector<engine::Entity>& GetMaterialEntities() const { return m_pMaterialStorage->GetEntities(); }
	const std::vector<engine::Entity>& GetNameEntities() const { return m_pNameStorage->GetEntities(); }
	const std::vector<engine::Entity>& GetSkyEntities() const { return m_pSkyStorage->GetEntities(); }
	const std::vector<engine::Entity>& GetStaticMeshEntities() const { return m_pStaticMeshStorage->GetEntities(); }
	const std::vector<engine::Entity>& GetTransformEntities() const { return m_pTransformStorage->GetEntities(); }

private:
	std::unique_ptr<cd::SceneDatabase> m_pSceneDatabase;
	std::unique_ptr<engine::World> m_pWorld;

	engine::ComponentsStorage<engine::CameraComponent>* m_pCameraStorage;
	engine::ComponentsStorage<engine::HierarchyComponent>* m_pHierarchyStorage;
	engine::ComponentsStorage<engine::LightComponent>* m_pLightStorage;
	engine::ComponentsStorage<engine::MaterialComponent>* m_pMaterialStorage;
	engine::ComponentsStorage<engine::NameComponent>* m_pNameStorage;
	engine::ComponentsStorage<engine::SkyComponent>* m_pSkyStorage;
	engine::ComponentsStorage<engine::StaticMeshComponent>* m_pStaticMeshStorage;
	engine::ComponentsStorage<engine::TransformComponent>* m_pTransformStorage;

	engine::Entity m_selectedEntity = engine::INVALID_ENTITY;
};

}