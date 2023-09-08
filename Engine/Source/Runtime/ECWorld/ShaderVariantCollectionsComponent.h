#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"

namespace engine
{

class ShaderVariantCollectionsComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("ShaderVariantCollectionsComponent");
		return className;
	}

	ShaderVariantCollectionsComponent() = default;
	ShaderVariantCollectionsComponent(const ShaderVariantCollectionsComponent&) = default;
	ShaderVariantCollectionsComponent& operator=(const ShaderVariantCollectionsComponent&) = default;
	ShaderVariantCollectionsComponent(ShaderVariantCollectionsComponent&&) = default;
	ShaderVariantCollectionsComponent& operator=(ShaderVariantCollectionsComponent&&) = default;
	~ShaderVariantCollectionsComponent() = default;

private:

};

}