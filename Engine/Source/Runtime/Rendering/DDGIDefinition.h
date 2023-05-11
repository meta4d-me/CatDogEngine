#pragma once

#include "Math/Vector.hpp"

namespace engine
{

enum class DDGITextureType
{
	Classification = 0,
	Distance,
	Irradiance,
	Relocation,

	Count,
};

constexpr const char* DDGITextureTypeName[] =
{
	"Classification",
	"Distance",
	"Irradiance",
	"Relocation",
};

static_assert(static_cast<int>(DDGITextureType::Count) == sizeof(DDGITextureTypeName) / sizeof(char *),
			  "DDGI texture type and names mismatch.");

CD_FORCEINLINE const char *GetDDGITextureTypeName(DDGITextureType type)
{
	return DDGITextureTypeName[static_cast<size_t>(type)];
}

}
