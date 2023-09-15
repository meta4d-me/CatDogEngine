#pragma once

#include "Material/MaterialType.h"
#include "Resources/ResourceBuilder.h"

#include <map>
#include <string>

namespace engine
{

class ShaderVariantCollections;

}

namespace editor
{

class ShaderBuilder
{
public:
	static void BuildNonUberShaders(engine::ShaderVariantCollections* pCollections);
	static void BuildUberShader(engine::ShaderVariantCollections* pCollections, engine::MaterialType* pMaterialType);

private:
	static const ShaderType GetShaderType(const std::string& fileName);
};

} // namespace editor
