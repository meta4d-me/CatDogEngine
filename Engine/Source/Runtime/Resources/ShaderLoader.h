#pragma once

#include "Material/MaterialType.h"

#include <map>
#include <string>

namespace engine
{

class ShaderLoader
{
public:
	static void UploadUberShader(engine::MaterialType* pMaterialType);
};

} // namespace editor
