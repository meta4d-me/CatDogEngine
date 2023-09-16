#pragma once

#include <map>
#include <string>

namespace engine
{

class MaterialType;
class RenderContext;
class ShaderVariantCollections;

class ShaderLoader
{
public:
	static void UploadUberShader(RenderContext* pRenderContext, ShaderVariantCollections* pShaderVariantCollections, MaterialType* pMaterialType);
	static void UploadNonUberShader(RenderContext* pRenderContext, ShaderVariantCollections* pShaderVariantCollections);
};

} // namespace editor
