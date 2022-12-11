#include "Core/StringCrc.h"
#include "EC$/CameraComponent.h"
#include "EC$/LightComponent.h"
#include "EC$/MaterialComponent.h"
#include "EC$/HierarchyComponent.h"
#include "EC$/World.h"
#include "EC$/StaticMeshComponent.h"
#include "EC$/TransformComponent.h"
#include "Utilities/PerformanceProfiler.h"

#include <cassert>
#include <random>
#include <set>

namespace
{

using namespace engine;

void Test_CreateEntity()
{
	cdtools::PerformanceProfiler perf("Test_CreateEntity");

	World world;

	constexpr int allocateCount = 1000;
	Entity entities[allocateCount];

#pragma omp parallel for
	for (int i = 0; i < allocateCount; ++i)
	{
		Entity entity = world.CreateEntity();
		entities[i] = entity;
	}

	std::set<Entity> uniqueEntities;
	for (Entity entity : entities)
	{
		uniqueEntities.insert(entity);
	}

	assert(uniqueEntities.size() == allocateCount);

	printf("[Success] Test_CreateEntity\n");
}

class Factory
{
public:
	ComponentsStorage<HierarchyComponent>* pHierarchy;
	ComponentsStorage<TransformComponent>* pTransform;
	ComponentsStorage<CameraComponent>* pCamera;
	ComponentsStorage<LightComponent>* pLight;
	ComponentsStorage<StaticMeshComponent>* pStaticMesh;
	ComponentsStorage<MaterialComponent>* pMaterial;
};

Factory Test_RegisterComponentStorages(World& world)
{
	cdtools::PerformanceProfiler perf("Test_RegisterComponentStorages");

	Factory factory;
	constexpr StringCrc smComponentName("StaticMeshComponent");
	factory.pStaticMesh = world.Register<StaticMeshComponent>(smComponentName);

	constexpr StringCrc materialComponentName("MaterialComponent");
	factory.pMaterial = world.Register<MaterialComponent>(materialComponentName);

	constexpr StringCrc lightComponentName("LightComponent");
	factory.pLight = world.Register<LightComponent>(lightComponentName);

	constexpr StringCrc transformComponentName("TransformComponent");
	factory.pTransform = world.Register<TransformComponent>(transformComponentName);

	constexpr StringCrc cameraComponentName("CameraComponent");
	factory.pCamera = world.Register<CameraComponent>(cameraComponentName);

	constexpr StringCrc hierarchyComponentName("HierarchyComponent");
	factory.pHierarchy = world.Register<HierarchyComponent>(hierarchyComponentName);

	printf("\n[Success] Test_RegisterComponentStorages\n");

	return factory;
}

std::vector<Entity> Test_CreateEntityComponents(World& world, Factory& factory)
{
	cdtools::PerformanceProfiler perf("Test_CreateEntityComponents");

	Entity levelEntity = world.CreateEntity();
	factory.pHierarchy->CreateComponent(levelEntity);

	constexpr int allocateCount = 100000;
	Entity meshEntites[allocateCount];

	// TODO : make it thread safe?
	//#pragma omp parallel for
	for (int i = 0; i < allocateCount; ++i)
	{
		Entity meshEntity = world.CreateEntity();
		factory.pHierarchy->CreateComponent(meshEntity);
		factory.pTransform->CreateComponent(meshEntity);
		factory.pStaticMesh->CreateComponent(meshEntity);
		factory.pMaterial->CreateComponent(meshEntity);

		meshEntites[i] = meshEntity;
	}

	assert(factory.pHierarchy->GetCount() == allocateCount + 1);
	assert(factory.pTransform->GetCount() == allocateCount);
	assert(factory.pStaticMesh->GetCount() == allocateCount);
	assert(factory.pMaterial->GetCount() == allocateCount);

	printf("\n[Success] Test_CreateEntityComponents\n");

	std::vector<Entity> entites;
	for (Entity meshEntity : meshEntites)
	{
		entites.push_back(meshEntity);
	}
	return entites;
}

void Test_RemoveEntityComponentsRandly(Factory& factory, const std::vector<Entity>& meshEntites)
{
	cdtools::PerformanceProfiler perf("Test_RemoveEntityComponentsRandly");

	size_t removeMeshCount = meshEntites.size() / 10;

	std::vector<size_t> randomIndexes;
	for (size_t i = removeMeshCount; i < removeMeshCount * 2; ++i)
	{
		randomIndexes.push_back(i);
	}

	uint32_t seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
	std::shuffle(randomIndexes.begin(), randomIndexes.end(), std::default_random_engine(seed));

	size_t oldHierachyCount = factory.pHierarchy->GetCount();
	size_t oldTransformCount = factory.pTransform->GetCount();
	size_t oldStaticMeshCount = factory.pStaticMesh->GetCount();
	size_t oldMaterialCount = factory.pMaterial->GetCount();

	for (size_t index : randomIndexes)
	{
		Entity meshEntity = meshEntites[index];
		factory.pHierarchy->RemoveComponent(meshEntity);
		factory.pTransform->RemoveComponent(meshEntity);
		factory.pStaticMesh->RemoveComponent(meshEntity);
		factory.pMaterial->RemoveComponent(meshEntity);
	}

	assert(oldHierachyCount - removeMeshCount == factory.pHierarchy->GetCount());
	assert(oldTransformCount - removeMeshCount == factory.pTransform->GetCount());
	assert(oldStaticMeshCount - removeMeshCount == factory.pStaticMesh->GetCount());
	assert(oldMaterialCount - removeMeshCount == factory.pMaterial->GetCount());

	assert(oldHierachyCount == factory.pHierarchy->GetCapcity());
	assert(oldTransformCount == factory.pTransform->GetCapcity());
	assert(oldStaticMeshCount == factory.pStaticMesh->GetCapcity());
	assert(oldMaterialCount == factory.pMaterial->GetCapcity());

	printf("\n[Success] Test_RemoveEntityComponentsRandly\n");
}

void Test_RemoveEntityComponentsByOrder(Factory& factory, const std::vector<Entity>& meshEntites)
{
	cdtools::PerformanceProfiler perf("Test_RemoveEntityComponentsByOrder");

	size_t removeMeshCount = meshEntites.size() / 10;
	size_t oldHierachyCount = factory.pHierarchy->GetCount();
	size_t oldTransformCount = factory.pTransform->GetCount();
	size_t oldStaticMeshCount = factory.pStaticMesh->GetCount();
	size_t oldMaterialCount = factory.pMaterial->GetCount();

	for (size_t i = removeMeshCount * 2; i < removeMeshCount * 3; ++i)
	{
		Entity meshEntity = meshEntites[i];
		factory.pHierarchy->RemoveComponent(meshEntity);
		factory.pTransform->RemoveComponent(meshEntity);
		factory.pStaticMesh->RemoveComponent(meshEntity);
		factory.pMaterial->RemoveComponent(meshEntity);
	}

	assert(oldHierachyCount - removeMeshCount == factory.pHierarchy->GetCount());
	assert(oldTransformCount - removeMeshCount == factory.pTransform->GetCount());
	assert(oldStaticMeshCount - removeMeshCount == factory.pStaticMesh->GetCount());
	assert(oldMaterialCount - removeMeshCount == factory.pMaterial->GetCount());

	assert(oldHierachyCount == factory.pHierarchy->GetCapcity());
	assert(oldTransformCount == factory.pTransform->GetCapcity());
	assert(oldStaticMeshCount == factory.pStaticMesh->GetCapcity());
	assert(oldMaterialCount == factory.pMaterial->GetCapcity());

	printf("\n[Success] Test_RemoveEntityComponentsByOrder\n");
}

void Test_CleanUnusedEntityComponents(Factory& factory)
{
	cdtools::PerformanceProfiler perf("Test_CleanUnusedEntityComponents");

	size_t oldHierachyCount = factory.pHierarchy->GetCount();
	size_t oldTransformCount = factory.pTransform->GetCount();
	size_t oldStaticMeshCount = factory.pStaticMesh->GetCount();
	size_t oldMaterialCount = factory.pMaterial->GetCount();

	factory.pHierarchy->CleanUnused();
	factory.pHierarchy->CleanUnused();
	factory.pTransform->CleanUnused();
	factory.pStaticMesh->CleanUnused();
	factory.pMaterial->CleanUnused();

	assert(oldHierachyCount == factory.pHierarchy->GetCount());
	assert(oldTransformCount == factory.pTransform->GetCount());
	assert(oldStaticMeshCount == factory.pStaticMesh->GetCount());
	assert(oldMaterialCount == factory.pMaterial->GetCount());

	assert(oldHierachyCount == factory.pHierarchy->GetCapcity());
	assert(oldTransformCount == factory.pTransform->GetCapcity());
	assert(oldStaticMeshCount == factory.pStaticMesh->GetCapcity());
	assert(oldMaterialCount == factory.pMaterial->GetCapcity());

	printf("\n[Success] Test_CleanUnusedEntityComponents\n");
}

}

int main()
{
	Test_CreateEntity();

	World world;
	Factory factory = Test_RegisterComponentStorages(world);
	std::vector<Entity> meshEntites = Test_CreateEntityComponents(world, factory);
	Test_RemoveEntityComponentsRandly(factory, meshEntites);
	Test_CleanUnusedEntityComponents(factory);
	Test_RemoveEntityComponentsByOrder(factory, meshEntites);
	Test_CleanUnusedEntityComponents(factory);

	return 0;
}