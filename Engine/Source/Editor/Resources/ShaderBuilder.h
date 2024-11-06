#pragma once

#include "Material/MaterialType.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Resources/ResourceBuilder.h"

#include <map>
#include <string>

namespace engine
{

class RenderContext;

}

namespace editor
{

class ShaderBuilder
{
public:
	static void RegisterUberShaderAllVariants(engine::RenderContext *pRenderContext, engine::MaterialType *pMaterialType);

	static void BuildRegisteredShaderResources(engine::RenderContext* pRenderContext, TaskOutputCallbacks callbacks = {});
	static void BuildRecompileShaderResources(engine::RenderContext* pRenderContext, TaskOutputCallbacks callbacks = {});
	static void BuildShaderResource(engine::RenderContext* pRenderContext, engine::ShaderResource* pShaderResource, TaskOutputCallbacks callbacks = {});
};

} // namespace editor
