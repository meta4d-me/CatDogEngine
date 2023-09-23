#include "ECWorldConsumer.h"

#include "ECWorld/AllComponentsHeader.h"
#include "ECWorld/SceneWorld.h"
#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Math/Transform.hpp"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ResourceLoader.h"
#include "Resources/ShaderBuilder.h"
#include "Scene/SceneDatabase.h"

#include <algorithm>
#include <filesystem>
//#include <format>

namespace editor
{

namespace Detail
{

const std::unordered_map<cd::MaterialTextureType, engine::ShaderFeature> materialTextureTypeToShaderFeature
{
	{ cd::MaterialTextureType::BaseColor, engine::ShaderFeature::ALBEDO_MAP },
	{ cd::MaterialTextureType::Normal, engine::ShaderFeature::NORMAL_MAP },
	{ cd::MaterialTextureType::Occlusion, engine::ShaderFeature::ORM_MAP },
	{ cd::MaterialTextureType::Roughness, engine::ShaderFeature::ORM_MAP },
	{ cd::MaterialTextureType::Metallic, engine::ShaderFeature::ORM_MAP },
	{ cd::MaterialTextureType::Emissive, engine::ShaderFeature::EMISSIVE_MAP },
};

CD_FORCEINLINE bool IsMaterialTextureTypeValid(cd::MaterialTextureType type)
{
	return materialTextureTypeToShaderFeature.find(type) != materialTextureTypeToShaderFeature.end();
}

} // namespace Detail

ECWorldConsumer::ECWorldConsumer(engine::SceneWorld* pSceneWorld, engine::RenderContext* pRenderContext) :
	m_pSceneWorld(pSceneWorld)
{
}

void ECWorldConsumer::SetSceneDatabaseIDs(uint32_t nodeID, uint32_t meshID)
{
	m_nodeMinID = nodeID;
	m_meshMinID = meshID;
}

void ECWorldConsumer::Execute(const cd::SceneDatabase* pSceneDatabase)
{
	if (0U == pSceneDatabase->GetMeshCount())
	{
		CD_WARN("[ECWorldConsumer] No valid meshes in the consumed SceneDatabase.");
	}

	auto ParseMesh = [&](cd::MeshID meshID, const cd::Transform& tranform)
	{
		engine::Entity meshEntity = m_pSceneWorld->GetWorld()->CreateEntity();
		AddTransform(meshEntity, tranform);

		const auto& mesh = pSceneDatabase->GetMesh(meshID.Data());

		// TODO : Or the user doesn't want to import animation data.
		const bool isStaticMesh = 0U == mesh.GetVertexInfluenceCount();
		if(isStaticMesh)
		{
			AddStaticMesh(meshEntity, mesh, m_pDefaultMaterialType->GetRequiredVertexFormat());

			cd::MaterialID meshMaterialID = mesh.GetMaterialID();
			AddMaterial(meshEntity, meshMaterialID.IsValid() ? &pSceneDatabase->GetMaterial(meshMaterialID.Data()) : nullptr, m_pDefaultMaterialType, pSceneDatabase);
		}
		else
		{
			engine::MaterialType* pMaterialType = m_pSceneWorld->GetAnimationMaterialType();
			AddSkinMesh(meshEntity, mesh, pMaterialType->GetRequiredVertexFormat());

			// TODO : Use a standalone .cdanim file to play animation.
			// Currently, we assume that imported SkinMesh will play animation automatically for testing.
			AddAnimation(meshEntity, pSceneDatabase->GetAnimation(0), pSceneDatabase);
			AddMaterial(meshEntity, nullptr, pMaterialType, pSceneDatabase);
		}
	};

	// There are multiple kinds of cases in the SceneDatabase:
	// 1. No nodes but have meshes in the SceneDatabase.
	// 2. Only a root node with multiple meshes.
	// 3. Node hierarchy.
	// Another case is that we want to skip Node/Mesh which alreay parsed previously.
	std::set<uint32_t> parsedMeshIDs;
	for (const auto& mesh : pSceneDatabase->GetMeshes())
	{
		if (m_meshMinID > mesh.GetID().Data())
		{
			continue;
		}

		ParseMesh(mesh.GetID(), cd::Transform::Identity());
		parsedMeshIDs.insert(mesh.GetID().Data());
	}

	for (const auto& node : pSceneDatabase->GetNodes())
	{
		if (m_nodeMinID > node.GetID().Data())
		{
			continue;
		}

		for (cd::MeshID meshID : node.GetMeshIDs())
		{
			if (parsedMeshIDs.find(meshID.Data()) != parsedMeshIDs.end())
			{
				continue;
			}

			ParseMesh(meshID, node.GetTransform());
		}
	}

	for (const auto& camera : pSceneDatabase->GetCameras())
	{
		engine::Entity cameraEntity = m_pSceneWorld->GetWorld()->CreateEntity();
		AddCamera(cameraEntity, camera);
	}

	for (const auto& light : pSceneDatabase->GetLights())
	{
		engine::Entity lightEntity = m_pSceneWorld->GetWorld()->CreateEntity();
		AddLight(lightEntity, light);
	}
}

void ECWorldConsumer::AddCamera(engine::Entity entity, const cd::Camera& camera)
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();
	engine::NameComponent& nameComponent = pWorld->CreateComponent<engine::NameComponent>(entity);
	nameComponent.SetName(camera.GetName());

	engine::CameraComponent& cameraComponent = pWorld->CreateComponent<engine::CameraComponent>(entity);
	const cd::Transform& cameraTransform = m_pSceneWorld->GetTransformComponent(entity)->GetTransform();
	cameraComponent.SetNearPlane(camera.GetNearPlane());
	cameraComponent.SetFarPlane(camera.GetFarPlane());
	cameraComponent.SetAspect(camera.GetAspect());
	cameraComponent.SetFov(camera.GetFov());
	cameraComponent.BuildProjectMatrix();
	cameraComponent.BuildViewMatrix(cameraTransform);
}

void ECWorldConsumer::AddLight(engine::Entity entity, const cd::Light& light)
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();
	auto& nameComponent = pWorld->CreateComponent<engine::NameComponent>(entity);
	nameComponent.SetName(light.GetName());

	auto& lightComponent = pWorld->CreateComponent<engine::LightComponent>(entity);
	lightComponent.SetType(light.GetType());
	lightComponent.SetIntensity(light.GetIntensity());
	lightComponent.SetRadius(light.GetRadius());
	lightComponent.SetRange(light.GetRange());
	lightComponent.SetWidth(light.GetWidth());
	lightComponent.SetHeight(light.GetHeight());
	lightComponent.SetAngleScale(light.GetAngleScale());
	lightComponent.SetAngleOffset(light.GetAngleOffset());
	lightComponent.SetColor(light.GetColor());
	lightComponent.SetPosition(light.GetPosition());
	lightComponent.SetDirection(light.GetDirection());
	lightComponent.SetUp(light.GetUp());

	auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
	transformComponent.SetTransform(cd::Transform::Identity());
	transformComponent.Build();
}

void ECWorldConsumer::AddTransform(engine::Entity entity, const cd::Transform& transform)
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();
	auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
	transformComponent.SetTransform(transform);
	transformComponent.Build();
}

void ECWorldConsumer::AddStaticMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat)
{
	assert(mesh.GetVertexCount() > 0 && mesh.GetPolygonCount() > 0);

	engine::World* pWorld = m_pSceneWorld->GetWorld();
	auto& nameComponent = pWorld->CreateComponent<engine::NameComponent>(entity);
	nameComponent.SetName(mesh.GetName());

	auto& collisionMeshComponent = pWorld->CreateComponent<engine::CollisionMeshComponent>(entity);
	collisionMeshComponent.SetType(engine::CollisonMeshType::AABB);
	collisionMeshComponent.SetAABB(mesh.GetAABB());
	collisionMeshComponent.Build();

	auto& staticMeshComponent = pWorld->CreateComponent<engine::StaticMeshComponent>(entity);
	staticMeshComponent.SetMeshData(&mesh);
	staticMeshComponent.SetRequiredVertexFormat(&vertexFormat);
	staticMeshComponent.Build();
	staticMeshComponent.Submit();
}

void ECWorldConsumer::AddSkinMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat)
{
	AddStaticMesh(entity, mesh, vertexFormat);
}

void ECWorldConsumer::AddAnimation(engine::Entity entity, const cd::Animation& animation, const cd::SceneDatabase* pSceneDatabase)
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();
	engine::AnimationComponent& animationComponent = pWorld->CreateComponent<engine::AnimationComponent>(entity);
	animationComponent.SetAnimationData(&animation);
	animationComponent.SetTrackData(pSceneDatabase->GetTracks().data());
	animationComponent.SetDuration(animation.GetDuration());
	animationComponent.SetTicksPerSecond(animation.GetTicksPerSecnod());

	bgfx::UniformHandle boneMatricesUniform = bgfx::createUniform("u_boneMatrices", bgfx::UniformType::Mat4, 128);
	animationComponent.SetBoneMatricesUniform(boneMatricesUniform.idx);
}

void ECWorldConsumer::AddMaterial(engine::Entity entity, const cd::Material* pMaterial, engine::MaterialType* pMaterialType, const cd::SceneDatabase* pSceneDatabase)
{
	std::set<uint8_t> compiledTextureSlot;
	std::map<std::string, const cd::Texture*> outputTexturePathToData;

	bool missRequiredTextures = false;
	bool unknownTextureSlot = false;
	if (pMaterial)
	{
		for (cd::MaterialTextureType requiredTextureType : pMaterialType->GetRequiredTextureTypes())
		{
			cd::TextureID textureID = pMaterial->GetTextureID(requiredTextureType);
			if (!textureID.IsValid())
			{
				missRequiredTextures = true;
				CD_ENGINE_ERROR("Material {0} massing required texture {1}!", pMaterial->GetName(),
					nameof::nameof_enum(requiredTextureType));
				break;
			}

			std::optional<uint8_t> optTextureSlot = pMaterialType->GetTextureSlot(requiredTextureType);
			if (!optTextureSlot.has_value())
			{
				unknownTextureSlot = true;
				CD_ENGINE_ERROR("Material {0} unknown texture slot of textuere type {1}!", pMaterial->GetName(),
					nameof::nameof_enum(requiredTextureType));
				break;
			}

			uint8_t textureSlot = optTextureSlot.value();
			const cd::Texture& requiredTexture = pSceneDatabase->GetTexture(textureID.Data());
			std::string outputTexturePath = engine::Path::GetTextureOutputFilePath(requiredTexture.GetPath(), ".dds");
			if (compiledTextureSlot.find(textureSlot) == compiledTextureSlot.end())
			{
				// When multiple textures have the same texture slot, it implies that these textures are packed in one file.
				// For example, AO + Metalness + Roughness are packed so they have same slots which mean we only need to build it once.
				// Note that these texture types can only have same setting to build texture.
				compiledTextureSlot.insert(textureSlot);
				ResourceBuilder::Get().AddTextureBuildTask(requiredTexture.GetType(), requiredTexture.GetPath(), outputTexturePath.c_str());
				outputTexturePathToData[cd::MoveTemp(outputTexturePath)] = &requiredTexture;
			}
		}
	}

	// In any bad case, we should have a material component.
	engine::MaterialComponent& materialComponent = m_pSceneWorld->GetWorld()->CreateComponent<engine::MaterialComponent>(entity);
	materialComponent.Init();
	materialComponent.SetMaterialType(pMaterialType);
	materialComponent.SetMaterialData(pMaterial);

	cd::Vec3f albedoColor(1.0f);
	engine::ShaderSchema& shaderSchema = pMaterialType->GetShaderSchema();
	if (missRequiredTextures || unknownTextureSlot)
	{
		// Give a special red color to notify.
		albedoColor = cd::Vec3f(1.0f, 0.0f, 0.0f);
	}
	else
	{
		if (pMaterial)
		{
			// Expected textures are ready to build. Add more optional texture data.
			for (cd::MaterialTextureType optionalTextureType : pMaterialType->GetOptionalTextureTypes())
			{
				cd::TextureID textureID = pMaterial->GetTextureID(optionalTextureType);
				if (!textureID.IsValid())
				{
					// TODO : Its ok to have a material factor instead of texture, remove factor case warning.
					CD_WARN("Material {0} does not have optional texture type {1}!", pMaterial->GetName(),
						nameof::nameof_enum(optionalTextureType));
					continue;
				}

				std::optional<uint8_t> optTextureSlot = pMaterialType->GetTextureSlot(optionalTextureType);
				if (!optTextureSlot.has_value())
				{
					CD_ERROR("Unknow texture {0} slot!", nameof::nameof_enum(optionalTextureType));
					break;
				}

				if (Detail::IsMaterialTextureTypeValid(optionalTextureType))
				{
					materialComponent.ActivateShaderFeature(Detail::materialTextureTypeToShaderFeature.at(optionalTextureType));
				}

				uint8_t textureSlot = optTextureSlot.value();
				const cd::Texture& optionalTexture = pSceneDatabase->GetTexture(textureID.Data());
				std::string outputTexturePath = engine::Path::GetTextureOutputFilePath(optionalTexture.GetPath(), ".dds");
				if (compiledTextureSlot.find(textureSlot) == compiledTextureSlot.end())
				{
					compiledTextureSlot.insert(textureSlot);
					ResourceBuilder::Get().AddTextureBuildTask(optionalTexture.GetType(), optionalTexture.GetPath(), outputTexturePath.c_str());
					outputTexturePathToData[cd::MoveTemp(outputTexturePath)] = &optionalTexture;
				}
			}

			if (auto optMetallic = pMaterial->GetFloatProperty(cd::MaterialPropertyGroup::Metallic, cd::MaterialProperty::Factor); optMetallic.has_value())
			{
				materialComponent.SetMetallicFactor(optMetallic.value());
			}

			if (auto optRoughness = pMaterial->GetFloatProperty(cd::MaterialPropertyGroup::Roughness, cd::MaterialProperty::Factor); optRoughness.has_value())
			{
				materialComponent.SetRoughnessFactor(optRoughness.value());
			}

			if (auto optTwoSided = pMaterial->GetBoolProperty(cd::MaterialPropertyGroup::General, cd::MaterialProperty::TwoSided); optTwoSided.has_value())
			{
				materialComponent.SetTwoSided(optTwoSided.value());
			}

			if (auto optBlendMode = pMaterial->GetI32Property(cd::MaterialPropertyGroup::General, cd::MaterialProperty::BlendMode); optBlendMode.has_value())
			{
				cd::BlendMode blendMode = static_cast<cd::BlendMode>(optBlendMode.value());
				if (cd::BlendMode::Mask == blendMode)
				{
					auto optAlphaTestValue = pMaterial->GetFloatProperty(cd::MaterialPropertyGroup::General, cd::MaterialProperty::OpacityMaskClipValue);
					assert(optAlphaTestValue.has_value());

					materialComponent.SetAlphaCutOff(optAlphaTestValue.value());
				}

				materialComponent.SetBlendMode(blendMode);
			}
		}
		else
		{
			albedoColor = cd::Vec3f(0.2f);
		}
	}

	// TODO : ResourceBuilder will move to EditorApp::Update in the future.
	// Now let's wait all resource build tasks done here.
	ResourceBuilder::Get().Update();

	// TODO : create material component before ResourceBuilder done.
	// Assign a special color for loading resource status.
	materialComponent.SetAlbedoColor(cd::MoveTemp(albedoColor));
	materialComponent.SetSkyType(m_pSceneWorld->GetSkyComponent(m_pSceneWorld->GetSkyEntity())->GetSkyType());

	// Textures.
	for (const auto& [outputTextureFilePath, pTextureData] : outputTexturePathToData)
	{
		auto textureFileBlob = engine::ResourceLoader::LoadFile(outputTextureFilePath.c_str());
		if(!textureFileBlob.empty())
		{
			materialComponent.AddTextureFileBlob(pTextureData->GetType(), pMaterial, *pTextureData, cd::MoveTemp(textureFileBlob));
		}
	}

	materialComponent.Build();
}

}