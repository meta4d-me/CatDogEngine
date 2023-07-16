#include "ShaderBuilder.h"

#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceLoader.h"

namespace editor
{

void ShaderBuilder::BuildUberShader(engine::MaterialType* pMaterialType)
{
	engine::ShaderSchema& shaderSchema = pMaterialType->GetShaderSchema();

	// No uber option support for VS now.
	std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetVertexShaderPath());
	ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Vertex,
		shaderSchema.GetVertexShaderPath(), outputVSFilePath.c_str());

	// Compile fragment shaders with uber options.
	for (const auto& combine : shaderSchema.GetUberCombines())
	{
		std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetFragmentShaderPath(), combine);
		ResourceBuilder::Get().AddShaderBuildTask(ShaderType::Fragment,
			shaderSchema.GetFragmentShaderPath(), outputFSFilePath.c_str(), combine.c_str());
	}

	CD_ENGINE_INFO("Material type {0} have shader variant count : {1}.", pMaterialType->GetMaterialName(), shaderSchema.GetUberCombines().size());
}

void ShaderBuilder::UploadUberShader(engine::MaterialType* pMaterialType)
{
	std::map<std::string, engine::StringCrc> outputFSPathToUberOption;

	engine::ShaderSchema& shaderSchema = pMaterialType->GetShaderSchema();
	std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetVertexShaderPath());
	for (const auto& combine : shaderSchema.GetUberCombines())
	{
		std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetFragmentShaderPath(), combine);
		outputFSPathToUberOption[cd::MoveTemp(outputFSFilePath)] = engine::StringCrc(combine);
	}
	CD_ENGINE_INFO("Material type {0} have shader variant count : {1}.", pMaterialType->GetMaterialName(), shaderSchema.GetUberCombines().size());

	// Vertex shader.
	shaderSchema.AddUberOptionVSBlob(ResourceLoader::LoadShader(outputVSFilePath.c_str()));
	const auto& VSBlob = shaderSchema.GetVSBlob();
	bgfx::ShaderHandle vsHandle = bgfx::createShader(bgfx::makeRef(VSBlob.data(), static_cast<uint32_t>(VSBlob.size())));
	bgfx::setName(vsHandle, outputVSFilePath.c_str());

	// Fragment shader.
	for (const auto& [outputFSFilePath, uberOptionCrc] : outputFSPathToUberOption)
	{
		shaderSchema.AddUberOptionFSBlob(uberOptionCrc, ResourceLoader::LoadShader(outputFSFilePath.c_str()));
	
		const auto& FSBlob = shaderSchema.GetFSBlob(uberOptionCrc);
		bgfx::ShaderHandle fsHandle = bgfx::createShader(bgfx::makeRef(FSBlob.data(), static_cast<uint32_t>(FSBlob.size())));
		bgfx::setName(fsHandle, outputFSFilePath.c_str());

		// Program.
		bgfx::ProgramHandle uberProgramHandle = bgfx::createProgram(vsHandle, fsHandle);
		shaderSchema.SetCompiledProgram(uberOptionCrc, uberProgramHandle.idx);
	}
}

void ShaderBuilder::BuildNonUberShader(std::string folderPath)
{
	for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath))
	{
		const auto& inputFilePath = entry.path();
		if (".sc" != inputFilePath.extension())
		{
			continue;
		}

		ShaderType shaderType = GetShaderType(inputFilePath.stem().string());
		if (shaderType == ShaderType::None)
		{
			continue;
		}

		std::string outputShaderPath = engine::Path::GetShaderOutputPath(inputFilePath.string().c_str());
		ResourceBuilder::Get().AddShaderBuildTask(shaderType,
			inputFilePath.string().c_str(), outputShaderPath.c_str());
	}
}

const ShaderType ShaderBuilder::GetShaderType(const std::string& fileName)
{
	if (fileName.starts_with("vs_") || fileName.starts_with("VS_"))
	{
		return ShaderType::Vertex;
	}
	else if (fileName.starts_with("fs_") || fileName.starts_with("FS_"))
	{
		return ShaderType::Fragment;
	}
	else if (fileName.starts_with("cs_") || fileName.starts_with("CS_"))
	{
		return ShaderType::Compute;
	}

	return ShaderType::None;
}

} // namespace editor
