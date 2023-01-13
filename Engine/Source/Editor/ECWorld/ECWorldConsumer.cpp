#include "ECWorldConsumer.h"

#include "ECWorld/ComponentsStorage.hpp"
#include "ECWorld/HierarchyComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/World.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Material/MaterialType.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceBuilder.h"
#include "Resources/ResourceLoader.h"
#include "Scene/Material.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/Node.h"
#include "Scene/SceneDatabase.h"
#include "Scene/VertexFormat.h"

#include <filesystem>
#include <format>

namespace editor
{

ECWorldConsumer::ECWorldConsumer(engine::World* pWorld, engine::MaterialType* pMaterialType, engine::RenderContext* pRenderContext) :
	m_pWorld(pWorld),
	m_pStandardMaterialType(pMaterialType),
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

	for (const auto& node : pSceneDatabase->GetNodes())
	{
		if (m_nodeMinID > node.GetID().Data())
		{
			// The SceneDatabase can be reused when we import assets multiple times.
			continue;
		}

		engine::Entity sceneEntity = m_pWorld->CreateEntity();
		AddNode(sceneEntity, node);

		for (cd::MeshID meshID : node.GetMeshIDs())
		{
			const auto& mesh = pSceneDatabase->GetMesh(meshID.Data());
			AddMesh(sceneEntity, mesh);

			cd::MaterialID meshMaterialID = mesh.GetMaterialID();
			if (meshMaterialID.IsValid())
			{
				const cd::Material& material = pSceneDatabase->GetMaterial(mesh.GetMaterialID().Data());
				AddMaterial(sceneEntity, material, pSceneDatabase);
			}

			m_meshEntities.push_back(sceneEntity);
		}
	}
}

void ECWorldConsumer::AddNode(engine::Entity entity, const cd::Node& node)
{
	engine::TransformComponent& transformComponent = m_pWorld->CreateComponent<engine::TransformComponent>(entity);
	transformComponent.SetTransform(node.GetTransform());

	engine::HierarchyComponent& hierarchyComponent = m_pWorld->CreateComponent<engine::HierarchyComponent>(entity);
	m_mapTransformIDToEntities[node.GetID().Data()] = entity;
	cd::NodeID parentTransformID = node.GetParentID();
	if (parentTransformID.Data() != cd::NodeID::InvalidID)
	{
		assert(m_mapTransformIDToEntities.contains(parentTransformID.Data()));
		hierarchyComponent.SetParentEntity(m_mapTransformIDToEntities[parentTransformID.Data()]);
	}
}

void ECWorldConsumer::AddMesh(engine::Entity entity, const cd::Mesh& mesh)
{
	assert(mesh.GetVertexCount() > 0 && mesh.GetPolygonCount() > 0);

	engine::NameComponent& nameComponent = m_pWorld->CreateComponent<engine::NameComponent>(entity);
	nameComponent.SetName(mesh.GetName());

	engine::StaticMeshComponent& staticMeshComponent = m_pWorld->CreateComponent<engine::StaticMeshComponent>(entity);
	staticMeshComponent.SetMeshData(&mesh);
	staticMeshComponent.SetRequiredVertexFormat(&m_pStandardMaterialType->GetRequiredVertexFormat());
}

std::string ECWorldConsumer::GetTextureOutputFilePath(const char* pInputFilePath)
{
	std::filesystem::path inputTexturePath(pInputFilePath);
	std::string inputTextureFileName = inputTexturePath.stem().generic_string();
	std::string outputTexturePath = CDENGINE_RESOURCES_ROOT_PATH;
	outputTexturePath += "Textures/" + inputTextureFileName + ".dds";
	return outputTexturePath;
}

void ECWorldConsumer::AddMaterial(engine::Entity entity, const cd::Material& material, const cd::SceneDatabase* pSceneDatabase)
{
	engine::MaterialComponent& materialComponent = m_pWorld->CreateComponent<engine::MaterialComponent>(entity);
	materialComponent.SetMaterialData(&material);
	materialComponent.SetMaterialType(m_pStandardMaterialType);

	std::map<std::string, const cd::Texture*> outputTexturePathToData;

	bool missRequiredTextures = false;
	for (cd::MaterialTextureType requiredTextureType : m_pStandardMaterialType->GetRequiredTextureTypes())
	{
		std::optional<cd::TextureID> optTexture = material.GetTextureID(requiredTextureType);
		if (!optTexture.has_value())
		{
			missRequiredTextures = true;
			break;
		}

		const cd::Texture& requiredTexture = pSceneDatabase->GetTexture(optTexture.value().Data());
		std::string outputTexturePath = GetTextureOutputFilePath(requiredTexture.GetPath());
		ResourceBuilder::Get().AddTextureBuildTask(requiredTexture.GetPath(), outputTexturePath.c_str(), requiredTexture.GetType());
		outputTexturePathToData[cd::MoveTemp(outputTexturePath)] = &requiredTexture;
	}

	if (missRequiredTextures)
	{
		// Missed required textures are unexpected behavior which will have a notification in the apperance with a special color.
		bgfx::ProgramHandle shadingProgram = m_pRenderContext->CreateProgram("MissingTextures",
			m_pStandardMaterialType->GetVertexShaderName(), "fs_missing_textures.bin");
		materialComponent.SetShadingProgram(shadingProgram.idx);
	}
	else
	{
		// Expected textures are ready to build. Add more optional texture data.
		for (cd::MaterialTextureType optionalTextureType : m_pStandardMaterialType->GetOptionalTextureTypes())
		{
			std::optional<cd::TextureID> optTexture = material.GetTextureID(optionalTextureType);
			if (!optTexture.has_value())
			{
				continue;
			}

			const cd::Texture& optionalTexture = pSceneDatabase->GetTexture(optTexture.value().Data());
			std::string outputTexturePath = GetTextureOutputFilePath(optionalTexture.GetPath());
			ResourceBuilder::Get().AddTextureBuildTask(optionalTexture.GetPath(), outputTexturePath.c_str(), optionalTexture.GetType());
			outputTexturePathToData[cd::MoveTemp(outputTexturePath)] = &optionalTexture;
		}

		bgfx::ProgramHandle shadingProgram = m_pRenderContext->CreateProgram(m_pStandardMaterialType->GetMaterialName(),
			m_pStandardMaterialType->GetVertexShaderName(), m_pStandardMaterialType->GetFragmentShaderName());
		materialComponent.SetShadingProgram(shadingProgram.idx);
	}

	// TODO : ResourceBuilder will move to EditorApp::Update in the future.
	// Now let's wait all resource build tasks done here.
	ResourceBuilder::Get().Update();

	for (const auto& [outputTextureFilePath, pTextureData] : outputTexturePathToData)
	{
		materialComponent.AddTextureBlob(pTextureData->GetType(), ResourceLoader::LoadTexture(outputTextureFilePath.c_str()));
	}
}

}