#include "ShaderLoader.h"

#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Rendering/ShaderType.h"
#include "Rendering/ShaderVariantCollections.h"
#include "Resources/ResourceLoader.h"

namespace engine
{

void ShaderLoader::UploadUberShader(RenderContext* pRenderContext, ShaderVariantCollections* pShaderVariantCollections, MaterialType* pMaterialType)
{
	//for (const auto& [programName, shaderNames] : pShaderVariantCollections->GetUberShaderPrograms())
	//{
	//	ShaderType type = ShaderType::None;
	//	std::string vsName;
	//	std::string fsName;
	//
	//	assert(shaderNames.size() <= 2);
	//	for (const auto& name : shaderNames)
	//	{
	//		ShaderType type = GetShaderType(name);
	//		if (ShaderType::Vertex == type)
	//		{
	//			vsName = name;
	//		}
	//		else if (ShaderType::Fragment == type)
	//		{
	//			fsName = name;
	//		}
	//		else
	//		{
	//			CD_ENGINE_WARN("Unknown shader type of {0}!", name);
	//		}
	//	}
	//
	//	if (!vsName.empty() && !fsName.empty())
	//	{
	//		std::string outputVSFilePath = Path::GetShaderOutputPath(vsName.c_str());
	//		std::vector<std::byte> vsData = ResourceLoader::LoadFile(outputVSFilePath.c_str());
	//		bgfx::ShaderHandle vsHandle = bgfx::createShader(bgfx::makeRef(vsData.data(), static_cast<uint32_t>(vsData.size())));
	//		bgfx::setName(vsHandle, outputVSFilePath.c_str());
	//
	//		for (const auto& featureCombine : pShaderVariantCollections->GetFeatureCombines(programName))
	//		{
	//			std::string outputFSFilePath = Path::GetShaderOutputPath(fsName.c_str(), featureCombine);
	//			std::vector<std::byte> fsData = ResourceLoader::LoadFile(outputFSFilePath.c_str());
	//			bgfx::ShaderHandle fsHandle = bgfx::createShader(bgfx::makeRef(vsData.data(), static_cast<uint32_t>(vsData.size())));
	//			bgfx::setName(fsHandle, outputFSFilePath.c_str());
	//			assert(bgfx::isValid(fsHandle));
	//
	//			// Pragram
	//			bgfx::ProgramHandle uberProgramHandle = bgfx::createProgram(vsHandle, fsHandle);
	//			assert(bgfx::isValid(uberProgramHandle));
	//			pShaderVariantCollections->SetNonUberShaderProgramHandle(programName, uberProgramHandle.idx);
	//		}
	//	}
	//	else
	//	{
	//		CD_ENGINE_WARN("Unknown program type of {0}!", programName);
	//	}
	//}

	{ // TODO : delete these
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
}

void ShaderLoader::UploadNonUberShader(RenderContext* pRenderContext, ShaderVariantCollections* pShaderVariantCollections)
{
	for (const auto& [programName, shaderNames] : pShaderVariantCollections->GetNonUberShaderPrograms())
	{
		ShaderType type = ShaderType::None;
		std::string vsName;
		std::string fsName;
		std::string csName;

		assert(shaderNames.size() <= 2);
		for (const auto& name : shaderNames)
		{
			ShaderType type = GetShaderType(name);
			if (ShaderType::Vertex == type)
			{
				vsName = name;
			}
			else if (ShaderType::Fragment == type)
			{
				fsName = name;
			}
			else if (ShaderType::Compute == type)
			{
				csName = name;
			}
			else
			{
				CD_ENGINE_WARN("Unknown shader type of {0}!", name);
			}
		}

		if (!vsName.empty() && !fsName.empty() && csName.empty())
		{
			bgfx::ProgramHandle programHandle = pRenderContext->CreateProgram(programName.c_str(), vsName.c_str(), fsName.c_str());
			assert(bgfx::isValid(programHandle));
			pShaderVariantCollections->SetNonUberShaderProgramHandle(programName, programHandle.idx);
		}
		else if (!csName.empty())
		{
			bgfx::ProgramHandle programHandle = pRenderContext->CreateProgram(programName.c_str(), csName.c_str());
			assert(bgfx::isValid(programHandle));
			pShaderVariantCollections->SetNonUberShaderProgramHandle(programName, programHandle.idx);
		}
		else
		{
			CD_ENGINE_WARN("Unknown program type of {0}!", programName);
		}
	}
}

} // namespace editor
