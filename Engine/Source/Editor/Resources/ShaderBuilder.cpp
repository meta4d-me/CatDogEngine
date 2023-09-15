#include "ShaderBuilder.h"

#include "ECWorld/SceneWorld.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/ShaderVariantCollections.h"

namespace editor
{

void ShaderBuilder::BuildUberShader(engine::ShaderVariantCollections* pCollections, engine::MaterialType* pMaterialType)
{
	const std::string& programName = pMaterialType->GetShaderSchema().GetProgramName();

	const std::set<std::string>& shaders = pCollections->GetUberShaders(programName);
	const std::set<std::string>& combines = pCollections->GetFeatureCombines(programName);
	for (const std::string& shader : shaders)
	{
		ShaderType shaderType = GetShaderType(shader);
		if (ShaderType::Vertex == shaderType)
		{
			std::string inputVSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
			std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shader.c_str());
			ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Vertex, inputVSFilePath.c_str(), outputVSFilePath.c_str());
		}
		else if (ShaderType::Fragment == shaderType)
		{
			for (const auto& combine : combines)
			{
				std::string inputVFSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
				std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shader.c_str(), combine);
				ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Fragment, inputVFSFilePath.c_str(), outputFSFilePath.c_str(), combine.c_str());
			}
		}
		else
		{
			// No shader feature support for vertex and compute shader.
			continue;
		}
	}

	engine::ShaderSchema& shaderSchema = pMaterialType->GetShaderSchema();

	// No shader feature support for VS now.
	std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetVertexShaderPath());
	ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Vertex,
		shaderSchema.GetVertexShaderPath(), outputVSFilePath.c_str());

	// Compile fragment shaders with shader features.
	for (const auto& combine : shaderSchema.GetFeatureCombines())
	{
		std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetFragmentShaderPath(), combine);
		ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Fragment,
			shaderSchema.GetFragmentShaderPath(), outputFSFilePath.c_str(), combine.c_str());
	}

	CD_ENGINE_INFO("Shader variant count of material type {0} : {1}", pMaterialType->GetMaterialName(), shaderSchema.GetFeatureCombines().size());
}

void ShaderBuilder::BuildNonUberShaders(engine::ShaderVariantCollections* pCollections)
{
	for (const auto& shaders : pCollections->GetNonUberShaderPrograms())
	{
		for (const auto& shader : shaders.second)
		{
			ShaderType shaderType = GetShaderType(shader);
			if (ShaderType::None == shaderType)
			{
				continue;
			}

			std::string inputShaderPath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
			std::string outputShaderPath = engine::Path::GetShaderOutputPath(shader.c_str());
			ResourceBuilder::Get().AddShaderBuildTask(shaderType, inputShaderPath.c_str(), outputShaderPath.c_str());
		}
	}
}

const ShaderType ShaderBuilder::GetShaderType(const std::string& fileName)
{
	if (fileName._Starts_with("vs_") || fileName._Starts_with("VS_"))
	{
		return ShaderType::Vertex;
	}
	else if (fileName._Starts_with("fs_") || fileName._Starts_with("FS_"))
	{
		return ShaderType::Fragment;
	}
	else if (fileName._Starts_with("cs_") || fileName._Starts_with("CS_"))
	{
		return ShaderType::Compute;
	}

	return ShaderType::None;
}

} // namespace editor
