#include "ECWorldConsumer.h"

#include "ECWorld/ComponentsStorage.hpp"
#include "ECWorld/HierarchyComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Material/MaterialType.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ResourceLoader.h"
#include "Resources/ShaderBuilder.h"
#include "Scene/SceneDatabase.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <span>

namespace editor
{

namespace Detail
{

const std::unordered_map<cd::MaterialTextureType, engine::Uber> materialTextureType2Uber
{
	// TODO : IBL
	{cd::MaterialTextureType::BaseColor, engine::Uber::ALBEDO},
	{cd::MaterialTextureType::Normal, engine::Uber::NORMAL_MAP},
	{cd::MaterialTextureType::Occlusion, engine::Uber::OCCLUSION},
	{cd::MaterialTextureType::Roughness, engine::Uber::ROUGHNESS},
	{cd::MaterialTextureType::Metallic, engine::Uber::METALLIC},
};

CD_FORCEINLINE bool IsMaterialTextureTypeValid(cd::MaterialTextureType type)
{
	return materialTextureType2Uber.find(type) != materialTextureType2Uber.end();
}

} // namespace Detail

ECWorldConsumer::ECWorldConsumer(engine::SceneWorld* pSceneWorld, engine::RenderContext* pRenderContext) :
	m_pSceneWorld(pSceneWorld),
	m_pRenderContext(pRenderContext)
{
}

void ECWorldConsumer::SetSceneDatabaseIDs(uint32_t nodeID)
{
	m_nodeMinID = nodeID;
}

void ECWorldConsumer::Execute(const cd::SceneDatabase* pSceneDatabase)
{
	assert(pSceneDatabase->GetMeshCount() > 0);
	
	ShaderBuilder::BuildUberShader(m_pSceneWorld->GetPBRMaterialType());

	for (const auto& node : pSceneDatabase->GetNodes())
	{
		if (m_nodeMinID > node.GetID().Data())
		{
			// The SceneDatabase can be reused when we import assets multiple times.
			continue;
		}

		for (cd::MeshID meshID : node.GetMeshIDs())
		{
			engine::Entity sceneEntity = m_pSceneWorld->GetWorld()->CreateEntity();
			AddNode(sceneEntity, node);

			const auto& mesh = pSceneDatabase->GetMesh(meshID.Data());

			// TODO : Or the user doesn't want to import animation data.
			const bool isStaticMesh = 0U == mesh.GetVertexInfluenceCount();
			if (isStaticMesh)
			{
				engine::MaterialType* pMaterialType = m_pSceneWorld->GetPBRMaterialType();
				AddStaticMesh(sceneEntity, mesh, pMaterialType->GetRequiredVertexFormat());

				cd::MaterialID meshMaterialID = mesh.GetMaterialID();
				if (meshMaterialID.IsValid())
				{
					AddMaterial(sceneEntity, &pSceneDatabase->GetMaterial(meshMaterialID.Data()), pMaterialType, pSceneDatabase);
				}
			}
			else
			{
				engine::MaterialType* pMaterialType = m_pSceneWorld->GetAnimationMaterialType();
				AddSkinMesh(sceneEntity, mesh, pMaterialType->GetRequiredVertexFormat());

				// TODO : Use a standalone .cdanim file to play animation.
				// Currently, we assume that imported SkinMesh will play animation automatically for testing.
				AddAnimation(sceneEntity, pSceneDatabase->GetAnimation(0), pSceneDatabase);
				AddMaterial(sceneEntity, nullptr, pMaterialType, pSceneDatabase);
			}
		}
	}
}

void ECWorldConsumer::AddNode(engine::Entity entity, const cd::Node& node)
{
	engine::World* pWorld = m_pSceneWorld->GetWorld();
	engine::TransformComponent& transformComponent = pWorld->CreateComponent<engine::TransformComponent>(entity);
	transformComponent.SetTransform(node.GetTransform());
	transformComponent.Build();
}

void ECWorldConsumer::AddStaticMesh(engine::Entity entity, const cd::Mesh& mesh, const cd::VertexFormat& vertexFormat)
{
	assert(mesh.GetVertexCount() > 0 && mesh.GetPolygonCount() > 0);

	engine::World* pWorld = m_pSceneWorld->GetWorld();
	engine::NameComponent& nameComponent = pWorld->CreateComponent<engine::NameComponent>(entity);
	nameComponent.SetName(mesh.GetName());

	engine::StaticMeshComponent& staticMeshComponent = pWorld->CreateComponent<engine::StaticMeshComponent>(entity);
	staticMeshComponent.SetMeshData(&mesh);
	staticMeshComponent.SetRequiredVertexFormat(&vertexFormat);
	staticMeshComponent.Build();
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
	for (cd::MaterialTextureType requiredTextureType : pMaterialType->GetRequiredTextureTypes())
	{
		std::optional<cd::TextureID> optTexture = pMaterial->GetTextureID(requiredTextureType);
		if (!optTexture.has_value())
		{
			missRequiredTextures = true;
			CD_ENGINE_ERROR("Material {0} massing required texture {1}!", pMaterial->GetName(),
				GetMaterialPropertyGroupName(requiredTextureType));
			break;
		}

		std::optional<uint8_t> optTextureSlot = pMaterialType->GetTextureSlot(requiredTextureType);
		if(!optTextureSlot.has_value())
		{
			unknownTextureSlot = true;
			CD_ENGINE_ERROR("Material {0} unknown texture slot of textuere type {1}!", pMaterial->GetName(), GetMaterialPropertyGroupName(requiredTextureType));
			break;
		}

		uint8_t textureSlot = optTextureSlot.value();
		const cd::Texture& requiredTexture = pSceneDatabase->GetTexture(optTexture.value().Data());
		std::string outputTexturePath = engine::Path::GetTextureOutputFilePath(requiredTexture.GetPath(), ".dds");
		if(!compiledTextureSlot.contains(textureSlot))
		{
			// When multiple textures have the same texture slot, it implies that these textures are packed in one file.
			// For example, AO + Metalness + Roughness are packed so they have same slots which mean we only need to build it once.
			// Note that these texture types can only have same setting to build texture.
			compiledTextureSlot.insert(textureSlot);
			ResourceBuilder::Get().AddTextureBuildTask(requiredTexture.GetType(), requiredTexture.GetPath(), outputTexturePath.c_str());
			outputTexturePathToData[cd::MoveTemp(outputTexturePath)] = &requiredTexture;
		}
	}

	engine::ShaderSchema& shaderSchema = pMaterialType->GetShaderSchema();
	engine::StringCrc currentUberOption(shaderSchema.GetUberCombines().at(0));
	if (missRequiredTextures || unknownTextureSlot)
	{
		// Treate missing resources case as a special uber option in the CPU side.
		currentUberOption = shaderSchema.GetProgramCrc(engine::LoadingStatus::MISSING_RESOURCES);
	}
	else
	{
		// Expected textures are ready to build. Add more optional texture data.
		for (cd::MaterialTextureType optionalTextureType : pMaterialType->GetOptionalTextureTypes())
		{
			std::optional<cd::TextureID> optTexture = pMaterial->GetTextureID(optionalTextureType);
			if (!optTexture.has_value())
			{
				CD_WARN("Material {0} does not have optional texture type {1}!", pMaterial->GetName(), GetMaterialPropertyGroupName(optionalTextureType));
				continue;
			}

			std::optional<uint8_t> optTextureSlot = pMaterialType->GetTextureSlot(optionalTextureType);
			if (!optTextureSlot.has_value())
			{
				unknownTextureSlot = true;
				CD_ERROR("Unknow texture {0} slot!", GetMaterialPropertyGroupName(optionalTextureType));
				break;
			}

			ActivateUberOption(optionalTextureType);

			uint8_t textureSlot = optTextureSlot.value();
			const cd::Texture& optionalTexture = pSceneDatabase->GetTexture(optTexture.value().Data());
			std::string outputTexturePath = engine::Path::GetTextureOutputFilePath(optionalTexture.GetPath(), ".dds");
			if (!compiledTextureSlot.contains(textureSlot))
			{
				compiledTextureSlot.insert(textureSlot);
				ResourceBuilder::Get().AddTextureBuildTask(optionalTexture.GetType(), optionalTexture.GetPath(), outputTexturePath.c_str());
				outputTexturePathToData[cd::MoveTemp(outputTexturePath)] = &optionalTexture;
			}
		}

		currentUberOption = shaderSchema.GetProgramCrc(m_activeUberOptions);
	}

	// TODO : ResourceBuilder will move to EditorApp::Update in the future.
	// Now let's wait all resource build tasks done here.
	ResourceBuilder::Get().Update();

	// TODO : create material component before ResourceBuilder done.
	// Assign a special color for loading resource status.
	engine::MaterialComponent& materialComponent = m_pSceneWorld->GetWorld()->CreateComponent<engine::MaterialComponent>(entity);
	materialComponent.SetMaterialData(pMaterial);
	materialComponent.SetMaterialType(pMaterialType);
	materialComponent.SetUberShaderOption(currentUberOption);

	// Textures.
	for (const auto& [outputTextureFilePath, pTextureData] : outputTexturePathToData)
	{
		auto textureBlob = ResourceLoader::LoadTexture(outputTextureFilePath.c_str());
		if(!textureBlob.empty())
		{
			materialComponent.AddTextureBlob(pTextureData->GetType(), cd::MoveTemp(textureBlob));
		}
	}

	materialComponent.Build();
}

void ECWorldConsumer::ActivateUberOption(cd::MaterialTextureType textureType)
{
	if (Detail::IsMaterialTextureTypeValid(textureType))
	{
		m_activeUberOptions.push_back(Detail::materialTextureType2Uber.at(textureType));
	}
	else
	{
		CD_WARN("MaterialTextureType {0} is not a vaild uber option!", GetMaterialPropertyGroupName(textureType));
	}
}

void ECWorldConsumer::DeactivateUberOption(cd::MaterialTextureType textureType)
{
	if (Detail::IsMaterialTextureTypeValid(textureType))
	{
		m_activeUberOptions.erase(std::find(m_activeUberOptions.begin(), m_activeUberOptions.end(), Detail::materialTextureType2Uber.at(textureType)));
	}
	else
	{
		CD_WARN("MaterialTextureType {0} is not a vaild uber option!", GetMaterialPropertyGroupName(textureType));
	}
}

void ECWorldConsumer::ClearActiveUberOption()
{
	m_activeUberOptions.clear();
}

}