#include "ShaderLoader.h"

#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceLoader.h"

namespace engine
{

void ShaderLoader::UploadUberShader(engine::MaterialType* pMaterialType)
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
	shaderSchema.AddUberOptionVSBlob(engine::ResourceLoader::LoadFile(outputVSFilePath.c_str()));
	const auto& VSBlob = shaderSchema.GetVSBlob();
	bgfx::ShaderHandle vsHandle = bgfx::createShader(bgfx::makeRef(VSBlob.data(), static_cast<uint32_t>(VSBlob.size())));
	bgfx::setName(vsHandle, outputVSFilePath.c_str());

	// Fragment shader.
	for (const auto& [outputFSFilePath, uberOptionCrc] : outputFSPathToUberOption)
	{
		shaderSchema.AddUberOptionFSBlob(uberOptionCrc, engine::ResourceLoader::LoadFile(outputFSFilePath.c_str()));
	
		const auto& FSBlob = shaderSchema.GetFSBlob(uberOptionCrc);
		bgfx::ShaderHandle fsHandle = bgfx::createShader(bgfx::makeRef(FSBlob.data(), static_cast<uint32_t>(FSBlob.size())));
		bgfx::setName(fsHandle, outputFSFilePath.c_str());
		assert(bgfx::isValid(fsHandle));

		// Program.
		bgfx::ProgramHandle uberProgramHandle = bgfx::createProgram(vsHandle, fsHandle);
		assert(bgfx::isValid(uberProgramHandle));
		shaderSchema.SetCompiledProgram(uberOptionCrc, uberProgramHandle.idx);
	}
}

} // namespace editor
