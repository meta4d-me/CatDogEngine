#include "ShaderBuilder.h"

#include "ECWorld/SceneWorld.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/ShaderVariantCollections.h"

namespace editor
{

void ShaderBuilder::BuildUberShader(engine::ShaderVariantCollections* pCollections, engine::MaterialType* pMaterialType)
{
	// const std::string& programName = pMaterialType->GetShaderSchema().GetProgramName();
	// 
	// const std::set<std::string>& shaders = pCollections->GetUberShaders(programName);
	// const std::set<std::string>& combines = pCollections->GetFeatureCombines(programName);
	// for (const std::string& shader : shaders)
	// {
	// 	engine::ShaderType shaderType = engine::GetShaderType(shader);
	// 	if (engine::ShaderType::Vertex == shaderType)
	// 	{
	// 		std::string inputVSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
	// 		std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shader.c_str());
	// 		ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Vertex, inputVSFilePath.c_str(), outputVSFilePath.c_str());
	// 	}
	// 	else if (engine::ShaderType::Fragment == shaderType)
	// 	{
	// 		for (const auto& combine : combines)
	// 		{
	// 			std::string inputVFSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
	// 			std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shader.c_str(), combine);
	// 			ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Fragment, inputVFSFilePath.c_str(), outputFSFilePath.c_str(), combine.c_str());
	// 		}
	// 	}
	// 	else
	// 	{
	// 		// No shader feature support for Vertex and Compute shader.
	// 		continue;
	// 	}
	// }

	{ // TODO : delete these
		engine::ShaderSchema& shaderSchema = pMaterialType->GetShaderSchema();

		std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetVertexShaderPath());
		ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Vertex,
			shaderSchema.GetVertexShaderPath(), outputVSFilePath.c_str());

		for (const auto& combine : shaderSchema.GetFeatureCombines())
		{
			std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetFragmentShaderPath(), combine);
			ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Fragment,
				shaderSchema.GetFragmentShaderPath(), outputFSFilePath.c_str(), combine.c_str());
		}

		CD_ENGINE_INFO("Shader variant count of material type {0} : {1}", pMaterialType->GetMaterialName(), shaderSchema.GetFeatureCombines().size());
	}
}

void ShaderBuilder::BuildNonUberShaders(engine::ShaderVariantCollections* pCollections)
{
	for (const auto& shaders : pCollections->GetNonUberShaderPrograms())
	{
		for (const auto& shader : shaders.second)
		{
			engine::ShaderType shaderType = engine::GetShaderType(shader);
			if (engine::ShaderType::None == shaderType)
			{
				continue;
			}

			std::string inputShaderPath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
			std::string outputShaderPath = engine::Path::GetShaderOutputPath(shader.c_str());
			ResourceBuilder::Get().AddShaderBuildTask(shaderType, inputShaderPath.c_str(), outputShaderPath.c_str());
		}
	}
}

} // namespace editor
