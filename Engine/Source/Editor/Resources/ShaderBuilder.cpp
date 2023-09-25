#include "ShaderBuilder.h"

#include "ECWorld/SceneWorld.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Rendering/ShaderCollections.h"

namespace editor
{

namespace
{

void CompileShaderVariants(const std::set<std::string>& shaders, const std::set<std::string>& combines)
{
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

}

void ShaderBuilder::CompileRegisteredNonUberShader(engine::RenderContext* pRenderContext)
{
	for (const auto& shaders : pRenderContext->GetShaderCollections()->GetShaderPrograms())
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

void ShaderBuilder::CompileRegisteredUberShader(engine::RenderContext* pRenderContext, engine::MaterialType* pMaterialType)
{
	const std::string& programName = pMaterialType->GetShaderSchema().GetProgramName();
	engine::StringCrc programNameCrc = engine::StringCrc(programName);

	if (!pRenderContext->GetShaderCollections()->HasFeatureCombine(programNameCrc))
	{
		return;
	}

	const std::set<std::string>& shaders = pRenderContext->GetShaderCollections()->GetShaders(programNameCrc);
	const std::set<std::string>& combines = pRenderContext->GetShaderCollections()->GetFeatureCombines(programNameCrc);
	CD_ENGINE_INFO("Compiling program {0} variant count : {1}", programName, combines.size());

	CompileShaderVariants(shaders, combines);
}

void ShaderBuilder::CompileUberShaderAllVariants(engine::RenderContext* pRenderContext, engine::MaterialType* pMaterialType)
{
	const std::string& programName = pMaterialType->GetShaderSchema().GetProgramName();

	const std::set<std::string>& shaders = pRenderContext->GetShaderCollections()->GetShaders(engine::StringCrc(programName));
	const std::set<std::string>& combines = pMaterialType->GetShaderSchema().GetAllFeatureCombines();
	CD_ENGINE_INFO("Compiling program {0} variant count : {1}", programName, combines.size());

	CompileShaderVariants(shaders, combines);

	ResourceBuilder::Get().Update();
}

void ShaderBuilder::BuildShader(engine::RenderContext* pRenderContext, const engine::ShaderCompileInfo& info)
{
	const std::set<std::string>& shaders = pRenderContext->GetShaderCollections()->GetShaders(engine::StringCrc(info.m_programName));

	for (const auto& shader : shaders)
	{
		engine::ShaderType shaderType = engine::GetShaderType(shader);
		if (engine::ShaderType::Vertex == shaderType || engine::ShaderType::Compute == shaderType)
		{
			std::string inputVSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
			std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shader.c_str());
			ResourceBuilder::Get().AddShaderBuildTask(shaderType, inputVSFilePath.c_str(), outputVSFilePath.c_str());
		}
		else if (engine::ShaderType::Fragment == shaderType)
		{
			std::string inputFSFilePath = engine::Path::GetBuiltinShaderInputPath(shader.c_str());
			std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shader.c_str(), info.m_featuresCombine);
			ResourceBuilder::Get().AddShaderBuildTask(engine::ShaderType::Fragment, inputFSFilePath.c_str(), outputFSFilePath.c_str(), info.m_featuresCombine.c_str());
		}
		else
		{
			continue;
		}
	}
}

} // namespace editor
