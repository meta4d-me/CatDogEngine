#include "ECWorldConsumer.h"

#include "ECWorld/AllComponentsHeader.h"
#include "ECWorld/SceneWorld.h"
#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Math/Transform.hpp"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/MeshResource.h"
#include "Rendering/Resources/ResourceContext.h"
#include "Rendering/Resources/TextureResource.h"
#include "Rendering/ShaderFeature.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ResourceLoader.h"
#include "Resources/ShaderBuilder.h"
#include "Scene/SceneDatabase.h"

#include <algorithm>
#include <filesystem>

namespace editor
{

ECWorldConsumer::ECWorldConsumer(engine::SceneWorld* pSceneWorld, engine::RenderContext* pRenderContext) :
	m_pSceneWorld(pSceneWorld), m_pRenderContext(pRenderContext), m_pResourceContext(pRenderContext->GetResourceContext())
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

		bool hasBlendShape = mesh.GetBlendShapeIDCount() > 0U;
		if (hasBlendShape)
		{
			assert(mesh.GetBlendShapeIDCount() == 1U);
			AddBlendShape(meshEntity, &mesh, pSceneDatabase->GetBlendShape(mesh.GetBlendShapeID(0U).Data()), pSceneDatabase);
		}

		bool hasSkin = mesh.GetSkinIDCount() > 0U;
		if (hasSkin)
		{
			engine::MaterialType* pMaterialType = m_pSceneWorld->GetAnimationMaterialType();
			AddSkinMesh(meshEntity, mesh, pMaterialType->GetRequiredVertexFormat());

			// TODO : Use a standalone .cdanim file to play animation.
			// Currently, we assume that imported SkinMesh will play animation automatically for testing.
			AddAnimation(meshEntity, pSceneDatabase->GetAnimation(0), pSceneDatabase);
			AddMaterial(meshEntity, nullptr, pMaterialType, pSceneDatabase);
		}
		else
		{
			AddStaticMesh(meshEntity, mesh, m_pDefaultMaterialType->GetRequiredVertexFormat());

			cd::MaterialID meshMaterialID = mesh.GetMaterialID(0U);
			AddMaterial(meshEntity, meshMaterialID.IsValid() ? &pSceneDatabase->GetMaterial(meshMaterialID.Data()) : nullptr, m_pDefaultMaterialType, pSceneDatabase);
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
	std::string meshName(mesh.GetName());
	engine::StringCrc meshNameCrc(meshName);
	nameComponent.SetName(cd::MoveTemp(meshName));

	auto& collisionMeshComponent = pWorld->CreateComponent<engine::CollisionMeshComponent>(entity);
	collisionMeshComponent.SetType(engine::CollisonMeshType::AABB);
	collisionMeshComponent.SetAABB(mesh.GetAABB());
	collisionMeshComponent.Build();

	auto& staticMeshComponent = pWorld->CreateComponent<engine::StaticMeshComponent>(entity);
	engine::MeshResource* pMeshResource = m_pResourceContext->AddMeshResource(meshNameCrc);
	pMeshResource->SetMeshAsset(&mesh);
	pMeshResource->UpdateVertexFormat(vertexFormat);
	staticMeshComponent.SetMeshResource(pMeshResource);
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
	animationComponent.SetTicksPerSecond(animation.GetTicksPerSecond());

	bgfx::UniformHandle boneMatricesUniform = bgfx::createUniform("u_boneMatrices", bgfx::UniformType::Mat4, 128);
	animationComponent.SetBoneMatricesUniform(boneMatricesUniform.idx);
}

void ECWorldConsumer::AddMaterial(engine::Entity entity, const cd::Material* pMaterial, engine::MaterialType* pMaterialType, const cd::SceneDatabase* pSceneDatabase)
{
	std::set<uint8_t> compiledTextureSlot;
	std::vector<std::tuple<cd::MaterialTextureType, std::string, const cd::Texture*>> outputTypeToData;

	engine::MaterialComponent& materialComponent = m_pSceneWorld->GetWorld()->CreateComponent<engine::MaterialComponent>(entity);
	materialComponent.Init();
	materialComponent.SetMaterialType(pMaterialType);
	materialComponent.SetMaterialData(pMaterial);
	materialComponent.ActivateShaderFeature(engine::GetSkyTypeShaderFeature(m_pSceneWorld->GetSkyComponent(m_pSceneWorld->GetSkyEntity())->GetSkyType()));

	// Expected textures are ready to build. Add more optional texture data.
	for (cd::MaterialTextureType optionalTextureType : pMaterialType->GetOptionalTextureTypes())
	{
		cd::TextureID textureID = pMaterial->GetTextureID(optionalTextureType);
		if (!textureID.IsValid())
		{
			continue;
		}

		std::optional<uint8_t> optTextureSlot = pMaterialType->GetTextureSlot(optionalTextureType);
		if (!optTextureSlot.has_value())
		{
			CD_ERROR("Unknow texture {0} slot!", nameof::nameof_enum(optionalTextureType));
			break;
		}

		uint8_t textureSlot = optTextureSlot.value();
		const cd::Texture& optionalTexture = pSceneDatabase->GetTexture(textureID.Data());
		std::string outputTexturePath = engine::Path::GetTextureOutputFilePath(optionalTexture.GetPath(), ".dds");
		if (compiledTextureSlot.find(textureSlot) == compiledTextureSlot.end())
		{
			// TODO : Resource level
			compiledTextureSlot.insert(textureSlot);
			ResourceBuilder::Get().AddTextureBuildTask(optionalTextureType, optionalTexture.GetPath(), outputTexturePath.c_str());
		}
		outputTypeToData.emplace_back(optionalTextureType, cd::MoveTemp(outputTexturePath), &optionalTexture);
	}

	// TODO : create material component before ResourceBuilder done.
	// Assign a special color for loading resource status.
	
	// TODO : Need pMaterial->GetVec3fProperty
	// if (auto optOcclusion = pMaterial->GetVec3fProperty(cd::MaterialPropertyGroup::BaseColor, cd::MaterialProperty::Factor); optOcclusion.has_value())
	// {
	//     materialComponent.SetFactor(cd::MaterialPropertyGroup::BaseColor, optOcclusion.value());
	// }
	
	if (auto optOcclusion = pMaterial->GetFloatProperty(cd::MaterialPropertyGroup::Occlusion, cd::MaterialProperty::Factor); optOcclusion.has_value())
	{
		materialComponent.SetFactor(cd::MaterialPropertyGroup::Occlusion, optOcclusion.value());
	}
	if (auto optRoughness = pMaterial->GetFloatProperty(cd::MaterialPropertyGroup::Roughness, cd::MaterialProperty::Factor); optRoughness.has_value())
	{
		materialComponent.SetFactor(cd::MaterialPropertyGroup::Metallic, optRoughness.value());
	}
	if (auto optMetallic = pMaterial->GetFloatProperty(cd::MaterialPropertyGroup::Metallic, cd::MaterialProperty::Factor); optMetallic.has_value())
	{
		materialComponent.SetFactor(cd::MaterialPropertyGroup::Metallic, optMetallic.value());
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

	// TODO : ResourceBuilder will move to EditorApp::Update in the future.
	// Now let's wait all resource build tasks done here.
	ResourceBuilder::Get().Update();

	// Textures.
	for (const auto& [type, path, pTexture] : outputTypeToData)
	{
		engine::TextureResource* pTextureResource = m_pResourceContext->AddTextureResource(engine::StringCrc(path));
		pTextureResource->SetTextureAsset(pTexture);
		pTextureResource->SetDDSBuiltTexturePath(path);
		pTextureResource->UpdateTextureType(type);
		pTextureResource->UpdateUVMapMode(pTexture->GetUMapMode(), pTexture->GetVMapMode());
		materialComponent.SetTextureResource(type, pMaterial, pTextureResource);

		if (auto pPropertyGroup = materialComponent.GetPropertyGroup(type); pPropertyGroup)
		{
			pPropertyGroup->useTexture = true;
			materialComponent.ActivateShaderFeature(engine::MaterialTextureTypeToShaderFeature.at(type));
		}
	}
}

void ECWorldConsumer::AddBlendShape(engine::Entity entity, const cd::Mesh* pMesh, const cd::BlendShape& blendShape, const cd::SceneDatabase* pSceneDatabase)
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();

	auto& blendShapeComponent = pWorld->CreateComponent<engine::BlendShapeComponent>(entity);

	uint32_t vertexInstanceCount = pMesh->GetVertexInstanceToIDCount();
	bool mappingSurfaceAttributes = vertexInstanceCount > 0U;
	if (mappingSurfaceAttributes)
	{
		// Hack : Clear instance data and copy vertex count.
		cd::Mesh* pModifiedMesh = const_cast<cd::Mesh*>(pMesh);

		std::map<uint32_t, std::vector<uint32_t>> vertexIDToInstances;
		std::vector<cd::Point> vertexPositions;
		vertexPositions.reserve(vertexInstanceCount);
		for (uint32_t vertexInstance = 0U; vertexInstance < vertexInstanceCount; ++vertexInstance)
		{
			uint32_t vertexID = pModifiedMesh->GetVertexInstanceToID(vertexInstance).Data();
			vertexIDToInstances[vertexID].push_back(vertexInstance);
			vertexPositions.emplace_back(pModifiedMesh->GetVertexPosition(vertexID));
		}
		
		for (cd::MorphID morphID : blendShape.GetMorphIDs())
		{
			const auto& morph = pSceneDatabase->GetMorph(morphID.Data());
			if (morph.GetVertexSourceIDCount() == 0U || morph.GetVertexPositionCount() == 0U)
			{
				continue;
			}

			std::vector<cd::VertexID> vertexSourceIDs;
			std::vector<cd::Point> morphedPositions;
			for (uint32_t vertexIndex = 0U; vertexIndex < morph.GetVertexSourceIDCount(); ++vertexIndex)
			{
				cd::VertexID sourceID = morph.GetVertexSourceID(vertexIndex);
				auto itInstanceIDs = vertexIDToInstances.find(sourceID.Data());
				assert(itInstanceIDs != vertexIDToInstances.end());
				const cd::Point& modifiedPosition = morph.GetVertexPosition(vertexIndex);
				for (uint32_t vertexInstance : itInstanceIDs->second)
				{
					vertexSourceIDs.push_back(vertexInstance);
					morphedPositions.push_back(modifiedPosition);
				}
			}

			auto& modifiedMorph = const_cast<cd::Morph&>(morph);
			modifiedMorph.SetVertexSourceIDs(cd::MoveTemp(vertexSourceIDs));
			modifiedMorph.SetVertexPositions(cd::MoveTemp(morphedPositions));
			blendShapeComponent.AddMorph(&modifiedMorph);
		}

		pModifiedMesh->SetVertexPositions(cd::MoveTemp(vertexPositions));
		pModifiedMesh->ClearVertexInstanceToIDs();
		blendShapeComponent.SetMesh(pModifiedMesh);
	}
	else
	{
		// Normal workflow.
		blendShapeComponent.SetMesh(pMesh);
		for (cd::MorphID morphID : blendShape.GetMorphIDs())
		{
			const auto& morph = pSceneDatabase->GetMorph(morphID.Data());
			if (morph.GetVertexSourceIDCount() == 0U || morph.GetVertexPositionCount() == 0U)
			{
				continue;
			}

			blendShapeComponent.AddMorph(&morph);
		}
	}

	blendShapeComponent.Build();
}

}