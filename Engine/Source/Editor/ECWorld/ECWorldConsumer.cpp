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
	staticMeshComponent.Build();
}

std::string ECWorldConsumer::GetShaderOutputFilePath(const char* pInputFilePath, const char* pAppendFileName)
{
	std::filesystem::path inputShaderPath(pInputFilePath);
	std::string inputShaderFileName = inputShaderPath.stem().generic_string();
	std::string outputShaderPath = CDENGINE_RESOURCES_ROOT_PATH;
	outputShaderPath += "Shaders/" + inputShaderFileName;
	if (pAppendFileName)
	{
		outputShaderPath += "_";
		outputShaderPath += pAppendFileName;
	}
	outputShaderPath += ".bin";
	return outputShaderPath;
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

	std::set<uint8_t> compiledTextureSlot;
	std::map<std::string, const cd::Texture*> outputTexturePathToData;
	std::map<std::string, engine::StringCrc> outputFSPathToUberOption;

	bool missRequiredTextures = false;
	bool unknownTextureSlot = false;
	for (cd::MaterialTextureType requiredTextureType : m_pStandardMaterialType->GetRequiredTextureTypes())
	{
		std::optional<cd::TextureID> optTexture = material.GetTextureID(requiredTextureType);
		if (!optTexture.has_value())
		{
			missRequiredTextures = true;
			break;
		}

		std::optional<uint8_t> optTextureSlot = m_pStandardMaterialType->GetTextureSlot(requiredTextureType);
		if(!optTextureSlot.has_value())
		{
			unknownTextureSlot = true;
			break;
		}

		uint8_t textureSlot = optTextureSlot.value();
		const cd::Texture& requiredTexture = pSceneDatabase->GetTexture(optTexture.value().Data());
		std::string outputTexturePath = GetTextureOutputFilePath(requiredTexture.GetPath());
		if(!compiledTextureSlot.contains(textureSlot))
		{
			// When multiple textures have the same texture slot, it implies that these textures are packed in one file.
			// For example, AO + Metalness + Roughness are packed so they have same slots which mean we only need to build it once.
			// Note that these texture types can only have same setting to build texture.
			compiledTextureSlot.insert(textureSlot);
			ResourceBuilder::Get().AddTextureBuildTask(requiredTexture.GetType(), requiredTexture.GetPath(), outputTexturePath.c_str());
		}
		outputTexturePathToData[cd::MoveTemp(outputTexturePath)] = &requiredTexture;
	}
	
	// No uber option support for VS now.
	engine::ShaderSchema& shaderSchema = m_pStandardMaterialType->GetShaderSchema();
	std::string outputVSFilePath = GetShaderOutputFilePath(shaderSchema.GetVertexShaderPath());
	ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Vertex,
		shaderSchema.GetVertexShaderPath(), outputVSFilePath.c_str());

	if (missRequiredTextures || unknownTextureSlot)
	{
		// Treate missing textures case as a special uber option in the CPU side.
		constexpr engine::StringCrc missingTextureOption("MissingTextures");
		if (!shaderSchema.IsUberOptionValid(missingTextureOption))
		{
			std::string inputFSShaderPath = CDENGINE_BUILTIN_SHADER_PATH;
			inputFSShaderPath += "fs_missing_textures.sc";

			std::string outputFSFilePath = GetShaderOutputFilePath(shaderSchema.GetFragmentShaderPath(), "MissingTextures");
			ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Fragment,
				inputFSShaderPath.c_str(), outputFSFilePath.c_str());

			std::string uberOptionName("MissingTextures");
			shaderSchema.RegisterUberOption(uberOptionName.c_str());

			engine::StringCrc uberOptionCrc(uberOptionName);
			outputFSPathToUberOption[cd::MoveTemp(outputFSFilePath)] = uberOptionCrc;
		}

		materialComponent.SetUberShaderOption(missingTextureOption);
	}
	else
	{
		// Expected textures are ready to build. Add more optional texture data.
		for (cd::MaterialTextureType optionalTextureType : m_pStandardMaterialType->GetOptionalTextureTypes())
		{
			std::optional<cd::TextureID> optTexture = material.GetTextureID(optionalTextureType);
			if (!optTexture.has_value())
			{
				// Optional texture is OK to failed to import.
				continue;
			}

			std::optional<uint8_t> optTextureSlot = m_pStandardMaterialType->GetTextureSlot(optionalTextureType);
			if (!optTextureSlot.has_value())
			{
				unknownTextureSlot = true;
				break;
			}

			uint8_t textureSlot = optTextureSlot.value();
			const cd::Texture& optionalTexture = pSceneDatabase->GetTexture(optTexture.value().Data());
			std::string outputTexturePath = GetTextureOutputFilePath(optionalTexture.GetPath());
			if (!compiledTextureSlot.contains(textureSlot))
			{
				compiledTextureSlot.insert(textureSlot);
				ResourceBuilder::Get().AddTextureBuildTask(optionalTexture.GetType(), optionalTexture.GetPath(), outputTexturePath.c_str());
			}
			outputTexturePathToData[cd::MoveTemp(outputTexturePath)] = &optionalTexture;
		}

		// Compile uber shaders with different options.
		std::vector<const char*> uberOptions;
		for (const auto& uberOption : shaderSchema.GetUberOptions())
		{
			// TODO : different compostions of uber options.
			// Currently, we only consider one uber option at the same time.
			uberOptions.clear();
			uberOptions.push_back(uberOption.c_str());
			std::string outputFSFilePath = GetShaderOutputFilePath(shaderSchema.GetFragmentShaderPath(), uberOption.c_str());
			ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Fragment,
				shaderSchema.GetFragmentShaderPath(), outputFSFilePath.c_str(), &uberOptions);

			engine::StringCrc uberOptionCrc(uberOption);
			outputFSPathToUberOption[cd::MoveTemp(outputFSFilePath)] = uberOptionCrc;
		}

		// At first, it will use default uber shader option.
		materialComponent.SetUberShaderOption(engine::ShaderSchema::DefaultUberOption);
	}

	// TODO : ResourceBuilder will move to EditorApp::Update in the future.
	// Now let's wait all resource build tasks done here.
	ResourceBuilder::Get().Update();

	for (const auto& [outputTextureFilePath, pTextureData] : outputTexturePathToData)
	{
		materialComponent.AddTextureBlob(pTextureData->GetType(), ResourceLoader::LoadTexture(outputTextureFilePath.c_str()));
	}

	shaderSchema.AddUberOptionVSBlob(ResourceLoader::LoadShader(outputVSFilePath.c_str()));
	const auto& VSBlob = shaderSchema.GetVSBlob();
	bgfx::ShaderHandle vsHandle = bgfx::createShader(bgfx::makeRef(VSBlob.data(), static_cast<uint32_t>(VSBlob.size())));

	for (const auto& [outputFSFilePath, uberOptionCrc] : outputFSPathToUberOption)
	{
		shaderSchema.AddUberOptionFSBlob(uberOptionCrc, ResourceLoader::LoadShader(outputFSFilePath.c_str()));
	
		const auto& FSBlob = shaderSchema.GetFSBlob(uberOptionCrc);
		bgfx::ShaderHandle fsHandle = bgfx::createShader(bgfx::makeRef(FSBlob.data(), static_cast<uint32_t>(FSBlob.size())));
		bgfx::ProgramHandle uberProgramHandle = bgfx::createProgram(vsHandle, fsHandle);
		shaderSchema.SetCompiledProgram(uberOptionCrc, uberProgramHandle.idx);
	}

	materialComponent.Build();
}

}