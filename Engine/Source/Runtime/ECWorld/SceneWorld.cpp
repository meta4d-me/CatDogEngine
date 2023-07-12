#include "SceneWorld.h"

#include "Log/Log.h"
#include "Path/Path.h"
#include "U_Slot.sh"
#include <iostream>
#ifdef ENABLE_DDGI_SDK
#include "ddgi_sdk.h"
#endif

namespace engine
{

SceneWorld::SceneWorld()
{
	m_pSceneDatabase = std::make_unique<cd::SceneDatabase>();

	m_pWorld = std::make_unique<engine::World>();

	// To add a new component : 2. Init component type here.
	m_pAnimationComponentStorage = m_pWorld->Register<engine::AnimationComponent>();
	m_pCameraComponentStorage = m_pWorld->Register<engine::CameraComponent>();
	m_pCollisionMeshComponentStorage = m_pWorld->Register<engine::CollisionMeshComponent>();
	m_pDDGIComponentStorage = m_pWorld->Register<engine::DDGIComponent>();
	m_pHierarchyComponentStorage = m_pWorld->Register<engine::HierarchyComponent>();
	m_pLightComponentStorage = m_pWorld->Register<engine::LightComponent>();
	m_pMaterialComponentStorage = m_pWorld->Register<engine::MaterialComponent>();
	m_pNameComponentStorage = m_pWorld->Register<engine::NameComponent>();
	m_pSkyComponentStorage = m_pWorld->Register<engine::SkyComponent>();
	m_pStaticMeshComponentStorage = m_pWorld->Register<engine::StaticMeshComponent>();
	m_pTransformComponentStorage = m_pWorld->Register<engine::TransformComponent>();

	CreatePBRMaterialType();
	CreateAnimationMaterialType();
	CreateTerrainMaterialType();
	CreateDDGIMaterialType();
}

void SceneWorld::CreatePBRMaterialType()
{
	m_pPBRMaterialType = std::make_unique<MaterialType>();
	m_pPBRMaterialType->SetMaterialName("CD_PBR");

	ShaderSchema shaderSchema(Path::GetBuiltinShaderInputPath("vs_PBR"), Path::GetBuiltinShaderInputPath("fs_PBR"));
	shaderSchema.RegisterUberOption(Uber::ALBEDO_MAP);
	shaderSchema.RegisterUberOption(Uber::NORMAL_MAP);
	shaderSchema.RegisterUberOption(Uber::ORM_MAP);
	shaderSchema.RegisterUberOption(Uber::EMISSIVE_MAP);
	// TODO : Revert it back after we can import CubeMap such as CmftStudio.
	//shaderSchema.RegisterUberOption(Uber::IBL);
	m_pPBRMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat pbrVertexFormat;
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pPBRMaterialType->SetRequiredVertexFormat(cd::MoveTemp(pbrVertexFormat));

	// Slot index should align to shader codes.
	// We want basic PBR materials to be flexible.
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::BaseColor, ALBEDO_MAP_SLOT);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Normal, NORMAL_MAP_SLOT);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Occlusion, ORM_MAP_SLOT);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Roughness, ORM_MAP_SLOT);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Metallic, ORM_MAP_SLOT);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Emissive, EMISSIVE_MAP_SLOT);
}

void SceneWorld::CreateAnimationMaterialType()
{
	m_pAnimationMaterialType = std::make_unique<MaterialType>();
	m_pAnimationMaterialType->SetMaterialName("CD_Animation");

	ShaderSchema shaderSchema(Path::GetBuiltinShaderInputPath("vs_animation"), Path::GetBuiltinShaderInputPath("fs_animation"));
	m_pAnimationMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat animationVertexFormat;
	animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	//animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::BoneIndex, cd::AttributeValueType::Int16, 4U);
	animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::BoneWeight, cd::AttributeValueType::Float, 4U);
	m_pAnimationMaterialType->SetRequiredVertexFormat(cd::MoveTemp(animationVertexFormat));
}

void SceneWorld::CreateTerrainMaterialType()
{
	m_pTerrainMaterialType = std::make_unique<MaterialType>();
	m_pTerrainMaterialType->SetMaterialName("CD_Terrain");

	ShaderSchema shaderSchema(Path::GetBuiltinShaderInputPath("vs_terrain"), Path::GetBuiltinShaderInputPath("fs_terrain"));
	shaderSchema.RegisterUberOption(Uber::DEFAULT);
	m_pTerrainMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat terrainVertexFormat;
	terrainVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	terrainVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pTerrainMaterialType->SetRequiredVertexFormat(cd::MoveTemp(terrainVertexFormat));

	m_pTerrainMaterialType->AddRequiredTextureType(cd::MaterialTextureType::Elevation, 1);
	m_pTerrainMaterialType->AddOptionalTextureType(cd::MaterialTextureType::AlphaMap, 2);
}

void SceneWorld::CreateDDGIMaterialType()
{
	m_pDDGIMaterialType = std::make_unique<MaterialType>();
	m_pDDGIMaterialType->SetMaterialName("CD_DDGI");

	ShaderSchema shaderSchema(Path::GetBuiltinShaderInputPath("vs_DDGI"), Path::GetBuiltinShaderInputPath("fs_DDGI"));
	m_pDDGIMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat ddgiVertexFormat;
	ddgiVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	ddgiVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	ddgiVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	ddgiVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pDDGIMaterialType->SetRequiredVertexFormat(cd::MoveTemp(ddgiVertexFormat));

	m_pDDGIMaterialType->AddRequiredTextureType(cd::MaterialTextureType::BaseColor, 0);
}

void SceneWorld::SetSelectedEntity(engine::Entity entity)
{
	CD_TRACE("Select entity : {0}", entity);
	m_selectedEntity = entity;
}

void SceneWorld::SetMainCameraEntity(engine::Entity entity)
{
	CD_TRACE("Setup main camera entity : {0}", entity);
	m_mainCameraEntity = entity;
}

void SceneWorld::SetDDGIEntity(engine::Entity entity)
{
	CD_TRACE("Setup DDGI entity : {0}", entity);
	m_ddgiEntity = entity;
}

void SceneWorld::SetPBRSkyEntity(engine::Entity entity)
{
	CD_TRACE("Setup PBRSky entity : {0}", entity);
	m_pbrskyEntity = entity;
}

void SceneWorld::AddCameraToSceneDatabase(engine::Entity entity)
{
	engine::CameraComponent* pCameraComponent = GetCameraComponent(entity);
	if (!pCameraComponent)
	{
		assert("Invalid entity");
		return;
	}

	std::string cameraName = "Untitled_Camera";
	if (const engine::NameComponent* pNameComponent = GetNameComponent(entity))
	{
		cameraName = pNameComponent->GetName();
	}

	cd::SceneDatabase* pSceneDatabase = GetSceneDatabase();
	cd::Camera camera(cd::CameraID(pSceneDatabase->GetCameraCount()), cameraName.c_str());
	camera.SetEye(pCameraComponent->GetEye());
	camera.SetLookAt(pCameraComponent->GetLookAt());
	camera.SetUp(pCameraComponent->GetUp());
	camera.SetNearPlane(pCameraComponent->GetNearPlane());
	camera.SetFarPlane(pCameraComponent->GetFarPlane());
	camera.SetAspect(pCameraComponent->GetAspect());
	camera.SetFov(pCameraComponent->GetFov());
	pSceneDatabase->AddCamera(cd::MoveTemp(camera));
}

void SceneWorld::AddLightToSceneDatabase(engine::Entity entity)
{
	engine::LightComponent* pLightComponent = GetLightComponent(entity);
	if (!pLightComponent)
	{
		assert("Invalid entity");
		return;
	}

	std::string lightName = "Untitled_Light";
	if (const engine::NameComponent* pNameComponent = GetNameComponent(entity))
	{
		lightName = pNameComponent->GetName();
	}

	cd::SceneDatabase* pSceneDatabase = GetSceneDatabase();
	cd::Light light(cd::LightID(pSceneDatabase->GetLightCount()), pLightComponent->GetType());
	light.SetName(lightName.c_str());
	light.SetIntensity(pLightComponent->GetIntensity());
	light.SetRange(pLightComponent->GetRange());
	light.SetRadius(pLightComponent->GetRadius());
	light.SetWidth(pLightComponent->GetWidth());
	light.SetHeight(pLightComponent->GetHeight());
	light.SetAngleScale(pLightComponent->GetAngleScale());
	light.SetAngleOffset(pLightComponent->GetAngleOffset());
	light.SetPosition(pLightComponent->GetPosition());
	light.SetColor(pLightComponent->GetColor());
	light.SetDirection(pLightComponent->GetDirection());
	light.SetUp(pLightComponent->GetUp());
	pSceneDatabase->AddLight(cd::MoveTemp(light));
}

void SceneWorld::Update(engine::Entity entity)
{
#ifdef ENABLE_DDGI_SDK
	engine::DDGIComponent* pDDGIComponent = GetDDGIComponent(entity);
	if (!pDDGIComponent)
	{
		assert("Invalid entity");
		return;
	}
	static uint32_t frameIndex = 1;
	static std::shared_ptr<CurrentFrameDecodeData> curDecodeData;
	std::this_thread::sleep_for(std::chrono::milliseconds(33));
	curDecodeData = GetCurDDGIFrameData();
	if (curDecodeData != nullptr)
	{
		pDDGIComponent->SetDistanceRawData(curDecodeData->visDecodeData);
		pDDGIComponent->SetIrradianceRawData(curDecodeData->irrDecodeData); 
	}
	++frameIndex;
#endif
}

void SceneWorld::InitSDK()
{
#ifdef ENABLE_DDGI_SDK
	std::string configFilePath = "C:/Users/V/Desktop/sdk/ddgi_sdk";
	if (InitDDGI(configFilePath))
	{
		std::cout << "Init DDGI Client Success!" << std::endl;
	}
	else
	{
		std::cout << "Init DDGI Client Failed!" << std::endl;
	}
#endif 
}
}