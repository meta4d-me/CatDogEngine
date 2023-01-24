#include "SceneWorld.h"

namespace engine
{

SceneWorld::SceneWorld()
{
	m_pSceneDatabase = std::make_unique<cd::SceneDatabase>();

	m_pWorld = std::make_unique<engine::World>();
	m_pCameraStorage = m_pWorld->Register<engine::CameraComponent>();
	m_pHierarchyStorage = m_pWorld->Register<engine::HierarchyComponent>();
	m_pLightStorage = m_pWorld->Register<engine::LightComponent>();
	m_pMaterialStorage = m_pWorld->Register<engine::MaterialComponent>();
	m_pNameStorage = m_pWorld->Register<engine::NameComponent>();
	m_pSkyStorage = m_pWorld->Register<engine::SkyComponent>();
	m_pStaticMeshStorage = m_pWorld->Register<engine::StaticMeshComponent>();
	m_pTransformStorage = m_pWorld->Register<engine::TransformComponent>();
}

}