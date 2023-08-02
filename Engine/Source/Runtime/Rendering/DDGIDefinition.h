#pragma once

#include "Math/Vector.hpp"

namespace engine
{

enum class DDGITextureType
{
	Distance = 0,
	Irradiance,
	Relocation,
	Classification,

	Count,
};

constexpr const char* DDGITextureTypeName[] =
{
	"Distance",
	"Irradiance",
	"Relocation",
	"Classification",
};

static_assert(static_cast<int>(DDGITextureType::Count) == sizeof(DDGITextureTypeName) / sizeof(char *),
			  "DDGI texture type and names mismatch.");

CD_FORCEINLINE const char *GetDDGITextureTypeName(DDGITextureType type)
{
	return DDGITextureTypeName[static_cast<size_t>(type)];
}

}
