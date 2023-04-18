#pragma once

#include "ECWorld/World.h"
#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Scene/SceneDatabase.h"

#include <memory>
#include <vector>

namespace engine
{

class MaterialType;

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

	void SetSelectedEntity(engine::Entity entity);
	CD_FORCEINLINE engine::Entity GetSelectedEntity() const { return m_selectedEntity; }

	void SetMainCameraEntity(engine::Entity entity);
	CD_FORCEINLINE engine::Entity GetMainCameraEntity() const { return m_mainCameraEntity; }

	void SetDDGIEntity(engine::Entity entity);
	CD_FORCEINLINE engine::Entity GetDDGIEntity() const { return m_ddgiEntity; }

	void OnResizeSceneView(uint16_t width, uint16_t height);

	void CreatePBRMaterialType();
	CD_FORCEINLINE engine::MaterialType* GetPBRMaterialType() const { return m_pPBRMaterialType.get(); }

	void CreateAnimationMaterialType();
	CD_FORCEINLINE engine::MaterialType* GetAnimationMaterialType() const { return m_pAnimationMaterialType.get(); }

	void CreateTerrainMaterialType();
	CD_FORCEINLINE engine::MaterialType* GetTerrainMaterialType() const { return m_pTerrainMaterialType.get(); }

	void CreateDDGIMaterialType();
	CD_FORCEINLINE engine::MaterialType *GetDDGIMaterialType() const { return m_pDDGIMaterialType.get(); }

	// It can save performance on addressing the actual ComponentStorage.
	// TODO : write a help macro ? Though I hate macro...
	CD_FORCEINLINE engine::AnimationComponent* GetAnimationComponent(engine::Entity entity) const { return m_pAnimationStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::CameraComponent* GetCameraComponent(engine::Entity entity) const { return m_pCameraStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::CollisionMeshComponent* GetCollisionMeshComponent(engine::Entity entity) const { return m_pCollisionMeshStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::DDGIComponent *GetDDGIComponent(engine::Entity entity) const { return m_pDDGIStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::HierarchyComponent* GetHierarchyComponent(engine::Entity entity) const { return m_pHierarchyStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::LightComponent* GetLightComponent(engine::Entity entity) const { return m_pLightStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::MaterialComponent* GetMaterialComponent(engine::Entity entity) const { return m_pMaterialStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::NameComponent* GetNameComponent(engine::Entity entity) const { return m_pNameStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::SkyComponent* GetSkyComponent(engine::Entity entity) const { return m_pSkyStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::StaticMeshComponent* GetStaticMeshComponent(engine::Entity entity) const { return m_pStaticMeshStorage->GetComponent(entity); }
	CD_FORCEINLINE engine::TransformComponent* GetTransformComponent(engine::Entity entity) const { return m_pTransformStorage->GetComponent(entity); }

	CD_FORCEINLINE const std::vector<engine::Entity>& GetAnimationEntities() const { return m_pAnimationStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetCameraEntities() const { return m_pCameraStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetCollisionMeshEntities() const { return m_pCollisionMeshStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity> &GetDDGIEntities() const { return m_pDDGIStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetHierarchyEntities() const { return m_pHierarchyStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetLightEntities() const { return m_pLightStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetMaterialEntities() const { return m_pMaterialStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetNameEntities() const { return m_pNameStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetSkyEntities() const { return m_pSkyStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetStaticMeshEntities() const { return m_pStaticMeshStorage->GetEntities(); }
	CD_FORCEINLINE const std::vector<engine::Entity>& GetTransformEntities() const { return m_pTransformStorage->GetEntities(); }

	void DeleteEntity(engine::Entity entity)
	{
		if (entity == m_mainCameraEntity)
		{
			CD_WARN("You can't delete main camera entity.");
			return;
		}

		if (entity == m_selectedEntity)
		{
			m_selectedEntity = engine::INVALID_ENTITY;
		}

		DeleteAnimationComponent(entity);
		DeleteCameraComponent(entity);
		DeleteCollisionMeshComponent(entity);
		DeleteDDGIComponent(entity);
		DeleteHierarchyComponent(entity);
		DeleteLightComponent(entity);
		DeleteMaterialComponent(entity);
		DeleteNameComponent(entity);
		DeleteSkyComponent(entity);
		DeleteStaticMeshComponent(entity);
		DeleteTransformComponent(entity);
	}

	CD_FORCEINLINE void DeleteAnimationComponent(engine::Entity entity) { m_pAnimationStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteCameraComponent(engine::Entity entity) { m_pCameraStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteCollisionMeshComponent(engine::Entity entity) { m_pCollisionMeshStorage->RemoveComponent(entity); }
	CD_FORCEINLINE void DeleteDDGIComponent(engine::Entity entity) { m_pDDGIStorage->RemoveComponent(entity); }
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

	std::unique_ptr<engine::MaterialType> m_pPBRMaterialType;
	std::unique_ptr<engine::MaterialType> m_pAnimationMaterialType;
	std::unique_ptr<engine::MaterialType> m_pTerrainMaterialType;
	std::unique_ptr<engine::MaterialType> m_pDDGIMaterialType;

	engine::ComponentsStorage<engine::AnimationComponent>* m_pAnimationStorage;
	engine::ComponentsStorage<engine::CameraComponent>* m_pCameraStorage;
	engine::ComponentsStorage<engine::CollisionMeshComponent>* m_pCollisionMeshStorage;
	engine::ComponentsStorage<engine::DDGIComponent> *m_pDDGIStorage;
	engine::ComponentsStorage<engine::HierarchyComponent>* m_pHierarchyStorage;
	engine::ComponentsStorage<engine::LightComponent>* m_pLightStorage;
	engine::ComponentsStorage<engine::MaterialComponent>* m_pMaterialStorage;
	engine::ComponentsStorage<engine::NameComponent>* m_pNameStorage;
	engine::ComponentsStorage<engine::SkyComponent>* m_pSkyStorage;
	engine::ComponentsStorage<engine::StaticMeshComponent>* m_pStaticMeshStorage;
	engine::ComponentsStorage<engine::TransformComponent>* m_pTransformStorage;

	// TODO : wrap them into another class?
	engine::Entity m_selectedEntity = engine::INVALID_ENTITY;
	engine::Entity m_mainCameraEntity = engine::INVALID_ENTITY;
	engine::Entity m_ddgiEntity = engine::INVALID_ENTITY;
};

}