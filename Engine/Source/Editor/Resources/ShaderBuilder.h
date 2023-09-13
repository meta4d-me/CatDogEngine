#pragma once

#include "Material/MaterialType.h"
#include "Resources/ResourceBuilder.h"

#include <map>
#include <string>

namespace engine
{

class SceneWorld;

}

namespace editor
{

class ShaderBuilder
{
public:
	static void BuildNonUberShader(std::string folderPath);
	static void BuildUberShader(engine::MaterialType* pMaterialType);

	static void BuildShaders(engine::SceneWorld* pSceneWorld);

private:
	static const ShaderType GetShaderType(const std::string& fileName);
};

} // namespace editor
