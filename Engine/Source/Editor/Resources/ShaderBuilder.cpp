#include "ShaderBuilder.h"

#include "ECWorld/SceneWorld.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/ShaderVariantCollections.h"

namespace editor
{

void ShaderBuilder::BuildUberShader(engine::MaterialType* pMaterialType)
{
	engine::ShaderSchema& shaderSchema = pMaterialType->GetShaderSchema();

	// No shader feature supports for VS now.
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

void ShaderBuilder::BuildShaders(engine::ShaderVariantCollections* pCollections)
{
	const auto& porgrams = pCollections->GetShaderPrograms();

	for (const auto& [programName, pack] : porgrams)
	{
		if (pack.GetFeatureSet().empty())
		{
			// Non-Uber Shaders

			for (const auto& shaderName : pack.GetShaderNames())
			{
				ShaderType shaderType = GetShaderType(shaderName);
				if (ShaderType::None == shaderType)
				{
					continue;
				}

				// TODO : GetBuiltinShaderInputPath()
				std::filesystem::path inputShaderFullPath = CDENGINE_BUILTIN_SHADER_PATH;
				inputShaderFullPath  += "shaders/";
				inputShaderFullPath  += shaderName;
				inputShaderFullPath .replace_extension(ShaderExtension);

				std::string outputShaderPath = engine::Path::GetShaderOutputPath(shaderName.c_str());
				ResourceBuilder::Get().AddShaderBuildTask(shaderType, inputShaderFullPath.generic_string().c_str(), outputShaderPath.c_str());
			}
		}
		else
		{
			// Uber Shaders

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
