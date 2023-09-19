#pragma once

#include "Material/MaterialType.h"
#include "Resources/ResourceBuilder.h"
#include "Rendering/ShaderVariantCompileInfo.h"

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
	static void BuildNonUberShaders(engine::RenderContext* pRenderContext);
	static void BuildUberShaders(engine::RenderContext* pRenderContext, engine::MaterialType* pMaterialType);

	// Compile specified uber shader program variant.
	static void BuildUberShader(engine::RenderContext* pRenderContext, engine::ShaderVariantCompileInfo info);
};

} // namespace editor
