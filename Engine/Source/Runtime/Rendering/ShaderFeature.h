#pragma once

#include <cstdint>
#include <set>

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
	ATM,
	AREAL_LIGHT,

	COUNT,
};

using ShaderFeatureSet = std::set<ShaderFeature>;

}