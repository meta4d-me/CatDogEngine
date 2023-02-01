#pragma once

#include "ECWorld/CameraComponent.h"
#include "ECWorld/CollisionMeshComponent.h"
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

class CollisionMeshComponent;
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

	CD_FORCEINLINE cd::SceneDatabase* GetSceneDatabase() { return m_pSceneDatabase.get(); }
	CD_FORCEINLINE engine::World* GetWorld() { return m_pWorld.get(); }
	CD_FORCEINLINE const engine::World* GetWorld() const { return m_pWorld.get(); }

	// Maybe not suitable to put selected entity here?
	CD_FORCEINLINE void SetSelectedEntity(engine::Entity entity) { m_selectedEntity = entity; }
	CD_FORCEINLINE engine::Entity GetSelectedEntity() const { return m_selectedEntity; }

	// It can save performance on addressing the actual ComponentStorage.
	// TODO : write a help macro ? Though I hate macro...
	CD_FORCEINLINE engine::CameraComponent* GetCameraComponent(engine::Entity entity) const { return m_pCameraStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::CollisionMeshComponent* GetCollisionMeshComponent(engine::Entity entity) const { return m_pCollisionMeshStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::HierarchyComponent* GetHierarchyComponent(engine::Entity entity) const { return m_pHierarchyStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::LightComponent* GetLightComponent(engine::Entity entity) const { return m_pLightStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::MaterialComponent* GetMaterialComponent(engine::Entity entity) const { return m_pMaterialStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::NameComponent* GetNameComponent(engine::Entity entity) const { return m_pNameStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::SkyComponent* GetSkyComponent(engine::Entity entity) const { return m_pSkyStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::StaticMeshComponent* GetStaticMeshComponent(engine::Entity entity) const { return m_pStaticMeshStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::TransformComponent* GetTransformComponent(engine::Entity entity) const { return m_pTransformStorage->GetComponent(entity); }

	CD_FORCEINLINE const std::vector<engine::Entity>& GetCameraEntities() const { return m_pCameraStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetCollisionMeshEntities() const { return m_pCollisionMeshStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetHierarchyEntities() const { return m_pHierarchyStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetLightEntities() const { return m_pLightStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetMaterialEntities() const { return m_pMaterialStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetNameEntities() const { return m_pNameStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetSkyEntities() const { return m_pSkyStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetStaticMeshEntities() const { return m_pStaticMeshStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetTransformEntities() const { return m_pTransformStorage->GetEntities(); }

	void DeleteEntity(engine::Entity entity)
	{
		DeleteCameraComponent(entity);
		DeleteCollisionMeshComponent(entity);
		DeleteHierarchyComponent(entity);
		DeleteLightComponent(entity);
		DeleteMaterialComponent(entity);
		DeleteNameComponent(entity);
		DeleteSkyComponent(entity);
		DeleteStaticMeshComponent(entity);
		DeleteTransformComponent(entity);
	}

	CD_FORCEINLINE void DeleteCameraComponent(engine::Entity entity) { m_pCameraStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteCollisionMeshComponent(engine::Entity entity) { m_pCollisionMeshStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteHierarchyComponent(engine::Entity entity) { m_pHierarchyStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteLightComponent(engine::Entity entity) { m_pLightStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteMaterialComponent(engine::Entity entity) { m_pMaterialStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteNameComponent(engine::Entity entity) { m_pNameStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteSkyComponent(engine::Entity entity) { m_pSkyStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteStaticMeshComponent(engine::Entity entity) { m_pStaticMeshStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteTransformComponent(engine::Entity entity) { m_pTransformStorage->RemoveComponent(entity); }

private:
	std::unique_ptr<cd::SceneDatabase> m_pSceneDatabase;
	std::unique_ptr<engine::World> m_pWorld;

	engine::ComponentsStorage<engine::CameraComponent>* m_pCameraStorage;
	engine::ComponentsStorage<engine::CollisionMeshComponent>* m_pCollisionMeshStorage;
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