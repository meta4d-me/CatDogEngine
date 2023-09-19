#include "ShaderBuilder.h"

#include "ECWorld/SceneWorld.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"

namespace editor
{

void ShaderBuilder::BuildUberShaders(engine::RenderContext* pRenderContext, engine::MaterialType* pMaterialType)
{
	const std::string& programName = pMaterialType->GetShaderSchema().GetProgramName();
	const std::set<std::string>& shaders = pRenderContext->GetShaderVariantCollections().GetUberShaders(programName);
	const std::set<std::string>& combines = pRenderContext->GetShaderVariantCollections().GetFeatureCombines(programName);
	CD_ENGINE_INFO("Precompile program {0} variant count : {1}", programName, combines.size());

	// CompileUberShader(shaders, combines);
	if (combines.size() <= 0)
	{
		return;
	}

	for (const std::string& shader : shaders)
	{
		engine::ShaderType shaderType = engine::GetShaderType(shader);
		if (engine::ShaderType::Vertex == shaderType)
		{
			std::string inputVSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
			std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shader.c_str());
			ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Vertex, inputVSFilePath.c_str(), outputVSFilePath.c_str());
		}
		else if (engine::ShaderType::Fragment == shaderType)
		{
			for (const auto& combine : combines)
			{
				std::string inputFSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
				std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shader.c_str(), combine);
				ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Fragment, inputFSFilePath.c_str(), outputFSFilePath.c_str(), combine.c_str());
			}
		}
		else
		{
			// No shader feature support for Vertex and Compute shader.
			continue;
		}
	}
}

void ShaderBuilder::BuildNonUberShaders(engine::RenderContext* pRenderContext)
{
	for (const auto& shaders : pRenderContext->GetShaderVariantCollections().GetNonUberShaderPrograms())
	{
		for (const auto& shader : shaders.second)
		{
			engine::ShaderType shaderType = engine::GetShaderType(shader);
			if (engine::ShaderType::None == shaderType)
			{
				CD_WARN("Known shader type of {0} in ShaderBuilder!", shader);
				continue;
			}

			std::string inputShaderPath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
			std::string outputShaderPath = engine::Path::GetShaderOutputPath(shader.c_str());
			ResourceBuilder::Get().AddShaderBuildTask(shaderType, inputShaderPath.c_str(), outputShaderPath.c_str());
		}
	}
}

void ShaderBuilder::BuildUberShader(engine::RenderContext* pRenderContext, engine::ShaderVariantCompileInfo info)
{
	std::string programName = cd::MoveTemp(info.m_programName);
	std::string combine = cd::MoveTemp(info.m_featuresCombine);
	const std::set<std::string>& shaders = pRenderContext->GetShaderVariantCollections().GetUberShaders(programName);

	for (const std::string& shader : shaders)
	{
		engine::ShaderType shaderType = engine::GetShaderType(shader);
		if (engine::ShaderType::Vertex == shaderType)
		{
			std::string inputVSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
			std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shader.c_str());
			ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Vertex, inputVSFilePath.c_str(), outputVSFilePath.c_str());
		}
		else if (engine::ShaderType::Fragment == shaderType)
		{
			std::string inputFSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
			std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shader.c_str(), combine);
			ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Fragment, inputFSFilePath.c_str(), outputFSFilePath.c_str(), combine.c_str());
		}
		else
		{
			// No shader feature support for Vertex and Compute shader.
			continue;
		}
	}
}

} // namespace editor
