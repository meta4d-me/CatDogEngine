#pragma once

#include "Material/MaterialType.h"
#include "Resources/ResourceBuilder.h"
#include "Rendering/ShaderCompileInfo.h"

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
	static void PreBuildNonUberShaders(engine::RenderContext* pRenderContext);
	static void PreBuildUberShaders(engine::RenderContext* pRenderContext, engine::MaterialType* pMaterialType);

	// Compile specified shader program/program variant.
	static void BuildShader(engine::RenderContext* pRenderContext, const engine::ShaderCompileInfo& info);
};

} // namespace editor
