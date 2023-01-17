#include "EditorSceneWorld.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/HierarchyComponent.h"
#include "ECWorld/LightComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/World.h"
#include "ECWorld/SkyComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"

namespace editor
{

EditorSceneWorld::EditorSceneWorld()
{
	m_pSceneDatabase = std::make_unique<cd::SceneDatabase>();

	m_pWorld = std::make_unique<engine::World>();
	m_pWorld->Register<engine::CameraComponent>();
	m_pWorld->Register<engine::HierarchyComponent>();
	m_pWorld->Register<engine::LightComponent>();
	m_pWorld->Register<engine::MaterialComponent>();
	m_pWorld->Register<engine::NameComponent>();
	m_pWorld->Register<engine::SkyComponent>();
	m_pWorld->Register<engine::StaticMeshComponent>();
	m_pWorld->Register<engine::TransformComponent>();
}

}