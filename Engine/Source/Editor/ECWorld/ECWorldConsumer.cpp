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
#include "Rendering/Resources/SkeletonResource.h"
#include "Rendering/Resources/TextureResource.h"
#include "Rendering/ShaderFeature.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ResourceLoader.h"
#include "Resources/ShaderBuilder.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
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
			// TODO : Use a standalone .cdanim file to play animation.
			// Currently, we assume that imported SkinMesh will play animation automatically for testing.
			//TODO : depens on skeleton to match animation
			if (pSceneDatabase->GetAnimationCount() <= 1U)
			{
				engine::MaterialType* pMaterialType = m_pSceneWorld->GetAnimationMaterialType();
				AddSkinMesh(meshEntity, mesh, pMaterialType->GetRequiredVertexFormat(), pSceneDatabase);
				AddAnimation(meshEntity, pSceneDatabase->GetAnimation(0), pSceneDatabase);
				AddMaterial(meshEntity, nullptr, pMaterialType, pSceneDatabase);
				AddSkeleton(meshEntity, pSceneDatabase);
			}
		}
		else
		{
			AddStaticMesh(meshEntity, mesh, m_pDefaultMaterialType->GetRequiredVertexFormat());

			cd::MaterialID meshMaterialID = mesh.GetMaterialID(0U);
			AddMaterial(meshEntity, meshMaterialID.IsValid() ? &pSceneDatabase->GetMaterial(meshMaterialID.Data()) : nullptr, m_pDefaultMaterialType, pSceneDatabase);
		}
	};

	// Parse particle emitter and skip its mesh shapes.
	std::set<cd::MeshID> parsedMeshIDs;
	for (auto& particleEmitter : pSceneDatabase->GetParticleEmitters())
	{
		engine::Entity emitterEntity = m_pSceneWorld->GetWorld()->CreateEntity();
		const auto& mesh = pSceneDatabase->GetMesh(particleEmitter.GetMeshID().Data());
		AddParticleEmitter(emitterEntity, mesh, m_pSceneWorld->GetParticleMaterialType()->GetRequiredVertexFormat(), particleEmitter);
		parsedMeshIDs.insert(mesh.GetID());
	}

	// Parse meshes in normal usage.
	for (const auto& mesh : pSceneDatabase->GetMeshes())
	{
		if (m_meshMinID > mesh.GetID().Data())
		{
			continue;
		}

		if (parsedMeshIDs.contains(mesh.GetID()))
		{
			continue;
		}

		ParseMesh(mesh.GetID(), cd::Transform::Identity());
		parsedMeshIDs.insert(mesh.GetID().Data());
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

void ECWorldConsumer::AddSkinMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat, const cd::SceneDatabase* pSceneDatabase)
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

	//auto& skinMeshComponent = pWorld->CreateComponent<engine::SkinMeshComponent>(entity);
	for (auto skinID : mesh.GetSkinIDs())
	{
		pMeshResource->SetSkinAsset(&pSceneDatabase->GetSkin(skinID.Data()));
		auto skeletonID = pSceneDatabase->GetSkin(skinID.Data()).GetSkeletonID();
		auto& skeleton = pSceneDatabase->GetSkeleton(skeletonID.Data());
		for (auto boneID : skeleton.GetBoneIDs())
		{
			pMeshResource->AddBonesAsset(pSceneDatabase->GetBone(boneID.Data()));
		}
	}

	pMeshResource->UpdateVertexFormat(vertexFormat);
	staticMeshComponent.SetMeshResource(pMeshResource);
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
	engine::MaterialComponent& materialComponent = m_pSceneWorld->GetWorld()->CreateComponent<engine::MaterialComponent>(entity);
	materialComponent.Init();
	materialComponent.SetMaterialType(pMaterialType);
	materialComponent.SetMaterialData(pMaterial);
	materialComponent.ActivateShaderFeature(engine::GetSkyTypeShaderFeature(m_pSceneWorld->GetSkyComponent(m_pSceneWorld->GetSkyEntity())->GetSkyType()));

	if (!pMaterial)
	{
		return;
	}

	std::set<uint8_t> compiledTextureSlot;
	std::vector<std::tuple<cd::MaterialTextureType, std::string, const cd::Texture*>> outputTypeToData;

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

		cd::Vec2f uvOffset;
		cd::Vec2f uvScale;
		if (auto optUVOffset = pMaterial->GetVec2fProperty(type, cd::MaterialProperty::UVOffset); optUVOffset.has_value())
		{
			uvOffset = optUVOffset.value();
		}
		if (auto optUVScale = pMaterial->GetVec2fProperty(type, cd::MaterialProperty::UVScale); optUVScale.has_value())
		{
			uvScale = optUVScale.value();
		}
		materialComponent.SetTextureResource(type, uvOffset, uvScale, pTextureResource);

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

void ECWorldConsumer::AddParticleEmitter(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat, const cd::ParticleEmitter& emitter)
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();
	engine::MaterialType* pMaterialType = m_pSceneWorld->GetParticleMaterialType();
	engine::NameComponent& nameComponent = pWorld->CreateComponent<engine::NameComponent>(entity);
	nameComponent.SetName(emitter.GetName());
	auto& particleEmitterComponent = pWorld->CreateComponent<engine::ParticleEmitterComponent>(entity);
	auto& particleMaterialComponent = pWorld->CreateComponent<engine::MaterialComponent>(entity);
	// TODO : Some initialization here.
	auto& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
	cd::Vec3f pos = emitter.GetPosition();
	cd::Vec3f rotation = emitter.GetFixedRotation();
	cd::Vec3f scale = emitter.GetFixedScale();
	auto fixedRotation = cd::Math::RadianToDegree(rotation);
	cd::Quaternion rotationQuat = cd::Quaternion::FromPitchYawRoll(fixedRotation.x(), fixedRotation.y(), fixedRotation.z());
	transformComponent.GetTransform().SetTranslation(pos);
	transformComponent.GetTransform().SetRotation(rotationQuat);
	transformComponent.GetTransform().SetScale(scale);
	transformComponent.Build();

	particleEmitterComponent.SetRequiredVertexFormat(&vertexFormat);
	////const cd::VertexFormat *requriredVertexFormat = emitter.GetVertexFormat();
	////particleEmitterComponent.SetRequiredVertexFormat(requriredVertexFormat);
	////particleEmitterComponent.GetParticleSystem().Init();
	if (nameof::nameof_enum(emitter.GetType()) == "Sprite") { particleEmitterComponent.SetEmitterParticleType(engine::ParticleType::Sprite); }
	else if (nameof::nameof_enum(emitter.GetType()) == "Ribbon") { particleEmitterComponent.SetEmitterParticleType(engine::ParticleType::Ribbon); }
	else if (nameof::nameof_enum(emitter.GetType()) == "Ring") { particleEmitterComponent.SetEmitterParticleType(engine::ParticleType::Ring); }
	else if (nameof::nameof_enum(emitter.GetType()) == "Model") { particleEmitterComponent.SetEmitterParticleType(engine::ParticleType::Model); }
	else if (nameof::nameof_enum(emitter.GetType()) == "Track") { particleEmitterComponent.SetEmitterParticleType(engine::ParticleType::Track); }

	particleEmitterComponent.SetSpawnCount(emitter.GetMaxCount());
	particleEmitterComponent.SetEmitterColor(emitter.GetColor()/255.0f);
	particleEmitterComponent.SetEmitterVelocity(emitter.GetVelocity());
	particleEmitterComponent.SetEmitterAcceleration(emitter.GetAccelerate());
	particleEmitterComponent.SetMeshData(&mesh); 
	particleMaterialComponent.SetMaterialType(pMaterialType);
	//particleMaterialComponent.ActivateShaderFeature(engine::ShaderFeature::PARTICLE_INSTANCE);
	particleEmitterComponent.Build();
}

void ECWorldConsumer::AddSkeleton(engine::Entity entity, const cd::SceneDatabase* pSceneDatabase)
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();
	auto& SkeletonComponent = pWorld->CreateComponent<engine::SkeletonComponent>(entity);
	
	auto& skeleton = pSceneDatabase->GetSkeleton(0);
	std::string meshName(skeleton.GetName());
	engine::StringCrc meshNameCrc(meshName);
	engine::SkeletonResource* pSkeletonResource = m_pResourceContext->AddSkeletonResource(meshNameCrc);
	pSkeletonResource->SetSceneDataBase(pSceneDatabase);
	SkeletonComponent.SetSkeletonAsset(pSkeletonResource);
	
}

}