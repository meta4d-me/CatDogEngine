#include "ShaderBuilder.h"

#include "ECWorld/SceneWorld.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ResourceContext.h"

namespace editor
{

void ShaderBuilder::RegisterUberShaderAllVariants(engine::RenderContext *pRenderContext, engine::MaterialType *pMaterialType)
{
	const std::string &programName = pMaterialType->GetShaderSchema().GetShaderProgramName();
	const engine::ShaderResource *pOriginShaderResource = pRenderContext->GetResourceContext()->GetShaderResource(engine::StringCrc{ programName });
	const auto &combines = pMaterialType->GetShaderSchema().GetAllFeatureCombines();
	engine::ShaderProgramType programType = pOriginShaderResource->GetType();

	for (const auto &combine : combines)
	{
		if (engine::ShaderProgramType::Standard == programType)
		{
			pRenderContext->RegisterShaderProgram(programName.c_str(),
				pOriginShaderResource->GetShaderInfo(0).name.c_str(),
				pOriginShaderResource->GetShaderInfo(0).name.c_str(),
				combine);
		}
		else
		{
			pRenderContext->RegisterShaderProgram(programName.c_str(),
				pOriginShaderResource->GetShaderInfo(0).name.c_str(),
				programType,
				combine);
		}
	}

	CD_ENGINE_INFO("Compiling program {0} all variants with count : {1}", programName, combines.size());
}

void ShaderBuilder::BuildRegisteredShaderResources(engine::RenderContext* pRenderContext, TaskOutputCallbacks callbacks)
{
	for (const auto& [_, pShaderResource] : pRenderContext->GetShaderResources())
	{
		BuildShaderResource(pRenderContext, pShaderResource, callbacks);
	}

	ResourceBuilder::Get().Update();
}

void ShaderBuilder::BuildRecompileShaderResources(engine::RenderContext* pRenderContext, TaskOutputCallbacks callbacks)
{
	for (auto pShaderResource : pRenderContext->GetRecompileShaderResources())
	{
		BuildShaderResource(pRenderContext, pShaderResource, callbacks);
	}

	ResourceBuilder::Get().Update();
	pRenderContext->ClearRecompileShaderResources();
}

void ShaderBuilder::BuildShaderResource(engine::RenderContext* pRenderContext, engine::ShaderResource* pShaderResource, TaskOutputCallbacks callbacks)
{
	if (engine::ShaderProgramType::Standard == pShaderResource->GetType())
	{
		const auto& vs = pShaderResource->GetShaderInfo(0);
		const auto& fs = pShaderResource->GetShaderInfo(1);

		TaskHandle vsTaskHandle = ResourceBuilder::Get().AddShaderBuildTask(vs.type,
			vs.scPath.c_str(), vs.binPath.c_str(), pShaderResource->GetFeaturesCombine().c_str(), callbacks);
		TaskHandle fsTaskHandle = ResourceBuilder::Get().AddShaderBuildTask(fs.type,
			fs.scPath.c_str(), fs.binPath.c_str(), pShaderResource->GetFeaturesCombine().c_str(), callbacks);
	}
	else
	{
		const auto& shader = pShaderResource->GetShaderInfo(0);
		TaskHandle taskHandle = ResourceBuilder::Get().AddShaderBuildTask(shader.type,
			shader.scPath.c_str(), shader.binPath.c_str(), pShaderResource->GetFeaturesCombine().c_str(), callbacks);
	}
}

} // namespace editor
