#pragma once

#include "Material/MaterialType.h"
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
	static void BuildNonUberShaders(engine::RenderContext* pRenderContext);
	static void BuildUberShader(engine::RenderContext* pRenderContext, engine::MaterialType* pMaterialType);
};

} // namespace editor
