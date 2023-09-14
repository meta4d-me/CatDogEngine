#include "ShaderLoader.h"

#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceLoader.h"

namespace engine
{

void ShaderLoader::UploadUberShader(engine::MaterialType* pMaterialType)
{
	std::map<std::string, engine::StringCrc> outputFSPathToShaderFeaturesCrc;

	engine::ShaderSchema& shaderSchema = pMaterialType->GetShaderSchema();
	std::string outputVSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetVertexShaderPath());
	for (const auto& combine : shaderSchema.GetFeatureCombines())
	{
		std::string outputFSFilePath = engine::Path::GetShaderOutputPath(shaderSchema.GetFragmentShaderPath(), combine);
		outputFSPathToShaderFeaturesCrc[cd::MoveTemp(outputFSFilePath)] = engine::StringCrc(combine);
	}

	// Vertex shader.
	shaderSchema.AddUberVSBlob(engine::ResourceLoader::LoadFile(outputVSFilePath.c_str()));
	const auto& VSBlob = shaderSchema.GetVSBlob();
	bgfx::ShaderHandle vsHandle = bgfx::createShader(bgfx::makeRef(VSBlob.data(), static_cast<uint32_t>(VSBlob.size())));
	bgfx::setName(vsHandle, outputVSFilePath.c_str());

	// Fragment shader.
	for (const auto& [outputFSFilePath, ShaderFeaturesCrc] : outputFSPathToShaderFeaturesCrc)
	{
		shaderSchema.AddUberFSBlob(ShaderFeaturesCrc, engine::ResourceLoader::LoadFile(outputFSFilePath.c_str()));
	
		const auto& FSBlob = shaderSchema.GetFSBlob(ShaderFeaturesCrc);
		bgfx::ShaderHandle fsHandle = bgfx::createShader(bgfx::makeRef(FSBlob.data(), static_cast<uint32_t>(FSBlob.size())));
		bgfx::setName(fsHandle, outputFSFilePath.c_str());
		assert(bgfx::isValid(fsHandle));

		// Program.
		bgfx::ProgramHandle uberProgramHandle = bgfx::createProgram(vsHandle, fsHandle);
		assert(bgfx::isValid(uberProgramHandle));
		shaderSchema.SetCompiledProgram(ShaderFeaturesCrc, uberProgramHandle.idx);
	}
}

} // namespace editor
