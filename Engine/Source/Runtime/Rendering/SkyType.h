#pragma once

#include "Base/Platform.h"
#include "Rendering/ShaderFeature.h"

#include <map>

namespace engine
{

enum class SkyType
{
	None,
	SkyBox,
	AtmosphericScattering,
};

constexpr engine::ShaderFeature SkyTypeToShaderFeature[] = {
	ShaderFeature::DEFAULT,
	ShaderFeature::IBL,
	ShaderFeature::ATM
};

constexpr engine::ShaderFeature GetSkyTypeShaderFeature(engine::SkyType type)
{
	return SkyTypeToShaderFeature[static_cast<size_t>(type)];
}

}