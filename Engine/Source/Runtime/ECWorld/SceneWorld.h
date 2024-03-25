#pragma once

#include "ECWorld/AllComponentsHeader.h"
#include "ECWorld/World.h"
#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Math/Transform.hpp"
#include "Scene/SceneDatabase.h"

#include <memory>
#include <vector>

namespace engine
{

class MaterialType;

// Helper macro to define a component type in the entity component world.
#define DEFINE_COMPONENT_STORAGE_WITH_APIS(ComponentType) \
private: \
	ComponentsStorage<ComponentType##Component>* m_p##ComponentType##ComponentStorage; \
public: \
	CD_FORCEINLINE const std::vector<engine::Entity>& Get##ComponentType##Entities() const { return m_p##ComponentType##ComponentStorage->GetEntities(); } \
	CD_FORCEINLINE ComponentType##Component* Get##ComponentType##Component(engine::Entity entity) const { return m_p##ComponentType##ComponentStorage->GetComponent(entity); } \
	CD_FORCEINLINE void Delete##ComponentType##Component(engine::Entity entity) { m_p##ComponentType##ComponentStorage->RemoveComponent(entity); }

class SceneWorld
{
	// To add a new component : 1. Define component type here.
	DEFINE_COMPONENT_STORAGE_WITH_APIS(Animation);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(BlendShape);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(Camera);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(CollisionMesh);
#ifdef ENABLE_DDGI
	DEFINE_COMPONENT_STORAGE_WITH_APIS(DDGI);
#endif
	DEFINE_COMPONENT_STORAGE_WITH_APIS(Hierarchy);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(Light);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(Material);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(Name);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(Sky);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(StaticMesh);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(ParticleEmitter);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(ParticleForceField);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(Terrain);
	DEFINE_COMPONENT_STORAGE_WITH_APIS(Transform);

public:
	SceneWorld();
	SceneWorld(const SceneWorld&) = delete;
	SceneWorld& operator=(const SceneWorld&) = delete;
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

#ifdef ENABLE_DDGI
	void SetDDGIEntity(engine::Entity entity);
	CD_FORCEINLINE engine::Entity GetDDGIEntity() const { return m_ddgiEntity; }
#endif

	void SetSkyEntity(engine::Entity entity);
	CD_FORCEINLINE engine::Entity GetSkyEntity() const { return m_skyEntity; }

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

		// To add a new component : 3. Delete component here when removing an entity.
		DeleteAnimationComponent(entity);
		DeleteBlendShapeComponent(entity);
		DeleteCameraComponent(entity);
		DeleteCollisionMeshComponent(entity);
#ifdef ENABLE_DDGI
		DeleteDDGIComponent(entity);
#endif
		DeleteHierarchyComponent(entity);
		DeleteLightComponent(entity);
		DeleteMaterialComponent(entity);
		DeleteNameComponent(entity);
		DeleteSkyComponent(entity);
		DeleteStaticMeshComponent(entity);
		DeleteParticleEmitterComponent(entity);
		DeleteParticleForceFieldComponent(entity);
		DeleteTerrainComponent(entity);
		DeleteTransformComponent(entity);
	}

	void CreatePBRMaterialType(std::string shaderProgramName, bool isAtmosphericScatteringEnable = false);
	CD_FORCEINLINE engine::MaterialType* GetPBRMaterialType() const { return m_pPBRMaterialType.get(); }

	void CreateAnimationMaterialType(std::string shaderProgramName);
	CD_FORCEINLINE engine::MaterialType* GetAnimationMaterialType() const { return m_pAnimationMaterialType.get(); }

	void CreateTerrainMaterialType(std::string shaderProgramName);
	CD_FORCEINLINE engine::MaterialType* GetTerrainMaterialType() const { return m_pTerrainMaterialType.get(); }

	void CreateParticleMaterialType(std::string shaderProgramName);
	CD_FORCEINLINE engine::MaterialType* GetParticleMaterialType() const { return m_pParticleMaterialType.get(); }

	void CreateCelluloidMaterialType(std::string shaderProgramName);
	CD_FORCEINLINE engine::MaterialType* GetCelluloidMaterialType() const { return m_pCelluloidMaterialType.get(); }

#ifdef ENABLE_DDGI
	void CreateDDGIMaterialType(std::string shaderProgramName);
	CD_FORCEINLINE engine::MaterialType* GetDDGIMaterialType() const { return m_pDDGIMaterialType.get(); }
#endif

	void AddCameraToSceneDatabase(engine::Entity entity);
	void AddLightToSceneDatabase(engine::Entity entity);
	void AddMaterialToSceneDatabase(engine::Entity entity);

#ifdef ENABLE_DDGI
	void InitDDGISDK();
#endif

	void Update();

private:
	std::unique_ptr<cd::SceneDatabase> m_pSceneDatabase;
	std::unique_ptr<engine::World> m_pWorld;

	std::unique_ptr<engine::MaterialType> m_pPBRMaterialType;
	std::unique_ptr<engine::MaterialType> m_pAnimationMaterialType;
	std::unique_ptr<engine::MaterialType> m_pCelluloidMaterialType;
	std::unique_ptr<engine::MaterialType> m_pTerrainMaterialType;
	std::unique_ptr<engine::MaterialType> m_pDDGIMaterialType;
	std::unique_ptr<engine::MaterialType> m_pPRMaterialType;
	std::unique_ptr<engine::MaterialType> m_pParticleMaterialType;

	// TODO : wrap them into another class?
	engine::Entity m_selectedEntity = engine::INVALID_ENTITY;
	engine::Entity m_mainCameraEntity = engine::INVALID_ENTITY;

	engine::Entity m_skyEntity = engine::INVALID_ENTITY;

#ifdef ENABLE_DDGI
	engine::Entity m_ddgiEntity = engine::INVALID_ENTITY;
#endif
};

}