#include "SceneWorld.h"

#include "Log/Log.h"
#include "Path/Path.h"
#include "U_BaseSlot.sh"
#include "U_Terrain.sh"

#ifdef ENABLE_DDGI
#include "ddgi_sdk.h"
#endif

#include <vector>
#include <string>

namespace engine
{

SceneWorld::SceneWorld()
{
	m_pSceneDatabase = std::make_unique<cd::SceneDatabase>();

	m_pWorld = std::make_unique<engine::World>();

	// To add a new component : 2. Init component type here.
	m_pAnimationComponentStorage = m_pWorld->Register<engine::AnimationComponent>();
	m_pBlendShapeComponentStorage = m_pWorld->Register<engine::BlendShapeComponent>();
	m_pCameraComponentStorage = m_pWorld->Register<engine::CameraComponent>();
	m_pCollisionMeshComponentStorage = m_pWorld->Register<engine::CollisionMeshComponent>();
#ifdef ENABLE_DDGI
	m_pDDGIComponentStorage = m_pWorld->Register<engine::DDGIComponent>();
#endif
	m_pHierarchyComponentStorage = m_pWorld->Register<engine::HierarchyComponent>();
	m_pLightComponentStorage = m_pWorld->Register<engine::LightComponent>();
	m_pMaterialComponentStorage = m_pWorld->Register<engine::MaterialComponent>();
	m_pNameComponentStorage = m_pWorld->Register<engine::NameComponent>();
	m_pSkyComponentStorage = m_pWorld->Register<engine::SkyComponent>();
	m_pStaticMeshComponentStorage = m_pWorld->Register<engine::StaticMeshComponent>();
	m_pParticleEmitterComponentStorage = m_pWorld->Register<engine::ParticleEmitterComponent>();
	m_pParticleForceFieldComponentStorage = m_pWorld->Register<engine::ParticleForceFieldComponent>();
	m_pTerrainComponentStorage = m_pWorld->Register<engine::TerrainComponent>();
	m_pTransformComponentStorage = m_pWorld->Register<engine::TransformComponent>();
	
#ifdef ENABLE_DDGI
	CreateDDGIMaterialType();
#endif
}

void SceneWorld::CreatePBRMaterialType(std::string shaderProgramName, bool isAtmosphericScatteringEnable)
{
	m_pPBRMaterialType = std::make_unique<MaterialType>();
	m_pPBRMaterialType->SetMaterialName("CD_PBR");

	ShaderSchema shaderSchema;
	shaderSchema.SetShaderProgramName(cd::MoveTemp(shaderProgramName));
	shaderSchema.AddFeatureSet({ ShaderFeature::ALBEDO_MAP });
	shaderSchema.AddFeatureSet({ ShaderFeature::NORMAL_MAP });
	shaderSchema.AddFeatureSet({ ShaderFeature::ORM_MAP });
	shaderSchema.AddFeatureSet({ ShaderFeature::EMISSIVE_MAP });
	// TODO : Compile atm shader in GL/VK mode correctly.
	isAtmosphericScatteringEnable ? shaderSchema.AddFeatureSet({ ShaderFeature::IBL, ShaderFeature::ATM }) : shaderSchema.AddFeatureSet({ ShaderFeature::IBL });
	shaderSchema.Build();
	m_pPBRMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat pbrVertexFormat;
	pbrVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	pbrVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
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

void SceneWorld::CreateAnimationMaterialType(std::string shaderProgramName)
{
	m_pAnimationMaterialType = std::make_unique<MaterialType>();
	m_pAnimationMaterialType->SetMaterialName("CD_Animation");

	ShaderSchema shaderSchema;
	shaderSchema.SetShaderProgramName(cd::MoveTemp(shaderProgramName));
	m_pAnimationMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat animationVertexFormat;
	animationVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	//animationVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	animationVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::BoneIndex, cd::AttributeValueType::Int16, 4U);
	animationVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::BoneWeight, cd::AttributeValueType::Float, 4U);
	m_pAnimationMaterialType->SetRequiredVertexFormat(cd::MoveTemp(animationVertexFormat));
}

void SceneWorld::CreateTerrainMaterialType(std::string shaderProgramName)
{
	m_pTerrainMaterialType = std::make_unique<MaterialType>();
	m_pTerrainMaterialType->SetMaterialName("CD_Terrain");

	ShaderSchema shaderSchema;
	shaderSchema.SetShaderProgramName(cd::MoveTemp(shaderProgramName));
	m_pTerrainMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat terrainVertexFormat;
	terrainVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	terrainVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	terrainVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	terrainVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pTerrainMaterialType->SetRequiredVertexFormat(cd::MoveTemp(terrainVertexFormat));
}

void SceneWorld::CreateCelluloidMaterialType(std::string shaderProgramName)
{
	m_pCelluloidMaterialType = std::make_unique<MaterialType>();
	m_pCelluloidMaterialType->SetMaterialName("CD_Celluloid");

	ShaderSchema shaderSchema;
	shaderSchema.SetShaderProgramName(cd::MoveTemp(shaderProgramName));
	shaderSchema.AddFeatureSet({ ShaderFeature::ALBEDO_MAP });
	shaderSchema.AddFeatureSet({ ShaderFeature::NORMAL_MAP });
	shaderSchema.AddFeatureSet({ ShaderFeature::EMISSIVE_MAP });
	m_pCelluloidMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat celluloidVertexFormat;
	celluloidVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	celluloidVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	celluloidVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	celluloidVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pCelluloidMaterialType->SetRequiredVertexFormat(cd::MoveTemp(celluloidVertexFormat));

	m_pCelluloidMaterialType->AddOptionalTextureType(cd::MaterialTextureType::BaseColor, ALBEDO_MAP_SLOT);
	m_pCelluloidMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Normal, NORMAL_MAP_SLOT);
	m_pCelluloidMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Emissive, EMISSIVE_MAP_SLOT);
}

void SceneWorld::CreateParticleMaterialType(std::string shaderProgramName)
{
	m_pParticleMaterialType = std::make_unique<MaterialType>();
	m_pParticleMaterialType->SetMaterialName("CD_Particle");

	ShaderSchema shaderSchema;
	shaderSchema.SetShaderProgramName(cd::MoveTemp(shaderProgramName));
	shaderSchema.AddFeatureSet({ ShaderFeature::PARTICLE_INSTANCE });
	shaderSchema.Build();
	m_pParticleMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat particleVertexFormat;
	particleVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	particleVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Color, cd::GetAttributeValueType<cd::Color::ValueType>(), cd::Color::Size);
	particleVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pParticleMaterialType->SetRequiredVertexFormat(cd::MoveTemp(particleVertexFormat));
}

#ifdef ENABLE_DDGI
void SceneWorld::CreateDDGIMaterialType(std::string shaderProgramName)
{
	m_pDDGIMaterialType = std::make_unique<MaterialType>();
	m_pDDGIMaterialType->SetMaterialName("CD_DDGI");

	ShaderSchema shaderSchema();
	shaderSchema.SetShaderProgramName(cd::MoveTemp(shaderProgramName));
	shaderSchema.AddFeatureSet({ ShaderFeature::ALBEDO_MAP });
	shaderSchema.AddFeatureSet({ ShaderFeature::NORMAL_MAP });
	shaderSchema.AddFeatureSet({ ShaderFeature::ORM_MAP });
	shaderSchema.AddFeatureSet({ ShaderFeature::EMISSIVE_MAP });
	shaderSchema.Build();
	m_pDDGIMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat ddgiVertexFormat;
	ddgiVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	ddgiVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	ddgiVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	ddgiVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pDDGIMaterialType->SetRequiredVertexFormat(cd::MoveTemp(ddgiVertexFormat));

	m_pDDGIMaterialType->AddOptionalTextureType(cd::MaterialTextureType::BaseColor, ALBEDO_MAP_SLOT);
	m_pDDGIMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Normal, NORMAL_MAP_SLOT);
	m_pDDGIMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Occlusion, ORM_MAP_SLOT);
	m_pDDGIMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Roughness, ORM_MAP_SLOT);
	m_pDDGIMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Metallic, ORM_MAP_SLOT);
	m_pDDGIMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Emissive, EMISSIVE_MAP_SLOT);
}
#endif

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

void SceneWorld::SetSkyEntity(engine::Entity entity)
{
	CD_TRACE("Setup Sky entity : {0}", entity);
	m_skyEntity = entity;
}

#ifdef ENABLE_DDGI
void SceneWorld::SetDDGIEntity(engine::Entity entity)
{
	CD_TRACE("Setup DDGI entity : {0}", entity);
	m_ddgiEntity = entity;
}
#endif

void SceneWorld::AddCameraToSceneDatabase(engine::Entity entity)
{
	engine::CameraComponent* pCameraComponent = GetCameraComponent(entity);
	cd::Transform CameraTransform = GetTransformComponent(entity)->GetTransform();
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
	camera.SetEye(CameraTransform.GetTranslation());
	//camera.SetLookAt(CameraTransform.GetLookAt());
	//camera.SetUp(CameraTransform.GetUp());
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

void SceneWorld::AddMaterialToSceneDatabase(engine::Entity entity)
{
	engine::MaterialComponent* pMaterialComponent = GetMaterialComponent(entity);
	assert(pMaterialComponent && "Invalid material entity");
	cd::Material* pMaterialData = pMaterialComponent->GetMaterialData();
	if (!pMaterialData)
	{
		return;
	}

	pMaterialData->SetVec3fProperty(cd::MaterialPropertyGroup::BaseColor, cd::MaterialProperty::Color, *(pMaterialComponent->GetFactor<cd::Vec3f>(cd::MaterialPropertyGroup::BaseColor)));
	pMaterialData->SetFloatProperty(cd::MaterialPropertyGroup::Metallic, cd::MaterialProperty::Factor, *(pMaterialComponent->GetFactor<float>(cd::MaterialPropertyGroup::Metallic)));
	pMaterialData->SetFloatProperty(cd::MaterialPropertyGroup::Roughness, cd::MaterialProperty::Factor, *(pMaterialComponent->GetFactor<float>(cd::MaterialPropertyGroup::Roughness)));
	pMaterialData->SetBoolProperty(cd::MaterialPropertyGroup::General, cd::MaterialProperty::TwoSided, pMaterialComponent->GetTwoSided());

	for (size_t textureTypeValue = 0; textureTypeValue < nameof::enum_count<cd::MaterialTextureType>(); ++textureTypeValue)
	{
		if (auto pPropertyGroup = pMaterialComponent->GetPropertyGroup(static_cast<cd::MaterialPropertyGroup>(textureTypeValue)); pPropertyGroup)
		{
			const MaterialComponent::TextureInfo& textureInfo = pPropertyGroup->textureInfo;
			pMaterialData->SetVec2fProperty(static_cast<cd::MaterialPropertyGroup>(textureTypeValue), cd::MaterialProperty::UVOffset, textureInfo.GetUVOffset());
			pMaterialData->SetVec2fProperty(static_cast<cd::MaterialPropertyGroup>(textureTypeValue), cd::MaterialProperty::UVScale, textureInfo.GetUVScale());
		}
	}
}

#ifdef ENABLE_DDGI
void SceneWorld::InitDDGISDK()
{
	if (InitDDGI(DDGI_SDK_PATH))
	{
		CD_ENGINE_FATAL("Init DDGI client success at : {0}", DDGI_SDK_PATH);
	}
	else
	{
		CD_ENGINE_FATAL("Init DDGI client failed at : {0}", DDGI_SDK_PATH);
	}
}
#endif

void SceneWorld::Update()
{
#ifdef ENABLE_DDGI
	// Send request 30 times per second.
	static auto startTime = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - startTime).count() <= 33 * 1000 * 1000)
	{
		return;
	}
	startTime = std::chrono::steady_clock::now();

	engine::DDGIComponent* pDDGIComponent = GetDDGIComponent(GetDDGIEntity());
	if (!pDDGIComponent)
	{
		CD_ENGINE_FATAL("Can not get DDGI component.");
		return;
	}

	std::shared_ptr<CurrentFrameDecodeData> curDecodeData = GetCurDDGIFrameData();
	if (curDecodeData != nullptr)
	{
		CD_ENGINE_FATAL("Receive DDGI raw data success.");

		// static uint32_t frameCount = 0;
		// static std::string savaPath = (std::filesystem::path(DDGI_SDK_PATH) / "Save").string();
		// WriteDdgi2BinFile(savaPath, *curDecodeData, frameCount++);

		pDDGIComponent->SetDistanceRawData(curDecodeData->visDecodeData);
		pDDGIComponent->SetIrradianceRawData(curDecodeData->irrDecodeData);
	}
#endif
}

}