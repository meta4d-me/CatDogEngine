#include "SceneWorld.h"

#include "Log/Log.h"

namespace
{

std::string GetShaderPath(const char* pShaderName)
{
	std::string shaderPath = CDENGINE_BUILTIN_SHADER_PATH;
	shaderPath += pShaderName;
	shaderPath += ".sc";

	return shaderPath;
}

}

namespace engine
{

SceneWorld::SceneWorld()
{
	m_pSceneDatabase = std::make_unique<cd::SceneDatabase>();

	m_pWorld = std::make_unique<engine::World>();
	m_pAnimationStorage = m_pWorld->Register<engine::AnimationComponent>();
	m_pCameraStorage = m_pWorld->Register<engine::CameraComponent>();
	m_pCollisionMeshStorage = m_pWorld->Register<engine::CollisionMeshComponent>();
	m_pHierarchyStorage = m_pWorld->Register<engine::HierarchyComponent>();
	m_pLightStorage = m_pWorld->Register<engine::LightComponent>();
	m_pMaterialStorage = m_pWorld->Register<engine::MaterialComponent>();
	m_pNameStorage = m_pWorld->Register<engine::NameComponent>();
	m_pSkyStorage = m_pWorld->Register<engine::SkyComponent>();
	m_pStaticMeshStorage = m_pWorld->Register<engine::StaticMeshComponent>();
	m_pTransformStorage = m_pWorld->Register<engine::TransformComponent>();

	CreatePBRMaterialType();
	CreateAnimationMaterialType();
}

void SceneWorld::CreatePBRMaterialType()
{
	m_pPBRMaterialType = std::make_unique<MaterialType>();
	m_pPBRMaterialType->SetMaterialName("CD_PBR");

	ShaderSchema shaderSchema(GetShaderPath("vs_PBR"), GetShaderPath("fs_PBR"));
	shaderSchema.RegisterUberOption(Uber::IBL);
	m_pPBRMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat pbrVertexFormat;
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	m_pPBRMaterialType->SetRequiredVertexFormat(cd::MoveTemp(pbrVertexFormat));

	// Slot index should align to shader codes.
	m_pPBRMaterialType->AddRequiredTextureType(cd::MaterialTextureType::BaseColor, 0);
	m_pPBRMaterialType->AddRequiredTextureType(cd::MaterialTextureType::Roughness, 2);
	m_pPBRMaterialType->AddRequiredTextureType(cd::MaterialTextureType::Metallic, 2);

	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Normal, 1);
	m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Occlusion, 2);
	//m_pPBRMaterialType->AddOptionalTextureType(cd::MaterialTextureType::Emissive, );
}

void SceneWorld::CreateAnimationMaterialType()
{
	m_pAnimationMaterialType = std::make_unique<MaterialType>();
	m_pAnimationMaterialType->SetMaterialName("CD_Animation");

	ShaderSchema shaderSchema(GetShaderPath("vs_animation"), GetShaderPath("fs_animation"));
	m_pAnimationMaterialType->SetShaderSchema(cd::MoveTemp(shaderSchema));

	cd::VertexFormat animationVertexFormat;
	animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	//animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::BoneIndex, cd::AttributeValueType::Int16, 4U);
	animationVertexFormat.AddAttributeLayout(cd::VertexAttributeType::BoneWeight, cd::AttributeValueType::Float, 4U);
	m_pAnimationMaterialType->SetRequiredVertexFormat(cd::MoveTemp(animationVertexFormat));
}

void SceneWorld::SetSelectedEntity(engine::Entity entity)
{
	CD_TRACE("Select entity : {0}", entity);
	m_selectedEntity = entity;
}

}