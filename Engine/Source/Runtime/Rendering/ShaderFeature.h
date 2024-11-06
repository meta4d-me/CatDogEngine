#pragma once

#include "Base/Platform.h"

#include <cstdint>
#include <map>
#include <set>
#include <vector>

namespace engine
{

enum class ShaderFeature : uint32_t
{
	DEFAULT = 0,

	// PBR parameters
	ALBEDO_MAP,
	NORMAL_MAP,
	ORM_MAP,
	EMISSIVE_MAP,

	// Techniques
	IBL,
	PARTICLE_INSTANCE,
	ATM,
	AREAL_LIGHT,

	COUNT,
};

// These names are used as macro definition symbols in shaders.
constexpr const char* ShaderFeatureNames[] =
{
	"", // Use empty string to represent default shader option in the name so we can reuse non-uber built shader.
	"ALBEDOMAP;",
	"NORMALMAP;",
	"ORMMAP;",
	"EMISSIVEMAP;",
	"IBL;",
	"PARTICLEINSTANCE;",
	"ATM;",
	"AREALLIGHT;",
};

static_assert(static_cast<int>(ShaderFeature::COUNT) == sizeof(ShaderFeatureNames) / sizeof(char*),
	"Shader features and names mismatch.");

constexpr const char* GetFeatureName(ShaderFeature feature)
{
	return ShaderFeatureNames[static_cast<size_t>(feature)];
}

using ShaderFeatureSet = std::set<ShaderFeature>;

}