#include "EditorWorld.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/LightComponent.h"
#include "ECWorld/HierarchyComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/World.h"
#include "ECWorld/SkyComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"

namespace editor
{

EditorWorld::EditorWorld()
{
	m_pWorld = std::make_unique<engine::World>();
	m_pWorld->Register<engine::CameraComponent>();
	m_pWorld->Register<engine::LightComponent>();
	m_pWorld->Register<engine::HierarchyComponent>();
	m_pWorld->Register<engine::MaterialComponent>();
	m_pWorld->Register<engine::SkyComponent>();
	m_pWorld->Register<engine::StaticMeshComponent>();
	m_pWorld->Register<engine::TransformComponent>();

	// Sky
	m_skyEntity = m_pWorld->CreateEntity();
	m_pWorld->CreateComponent<engine::SkyComponent>(m_skyEntity);
	m_pWorld->CreateComponent<engine::StaticMeshComponent>(m_skyEntity);

	// Meshes
	m_meshEntites.reserve(1024);
}

}