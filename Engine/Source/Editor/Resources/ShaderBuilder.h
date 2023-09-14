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
	static constexpr const char* ShaderExtension = ".sc";

public:
	static void BuildUberShader(engine::MaterialType* pMaterialType);

	static void BuildShaders(engine::ShaderVariantCollections* pCollections);

private:
	static const ShaderType GetShaderType(const std::string& fileName);
};

} // namespace editor
