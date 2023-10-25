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

static const std::map<engine::SkyType, engine::ShaderFeature> SkyTypeToShaderFeature
{
	{ engine::SkyType::None, engine::ShaderFeature::DEFAULT},
	{ engine::SkyType::SkyBox, engine::ShaderFeature::IBL},
	{ engine::SkyType::AtmosphericScattering, engine::ShaderFeature::ATM },
};

CD_FORCEINLINE engine::ShaderFeature GetSkyTypeShaderFeature(engine::SkyType type)
{
	return SkyTypeToShaderFeature.at(type);
}

}