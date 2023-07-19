#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"

namespace engine
{

enum class SkyType
{
	SkyBox = 0,
	AtmosphericScattering,

	Count,
};

constexpr const char *SkyTypeName[] =
{
	"Skybox",
	"Atmospheric Scattering",
};

static_assert(static_cast<int>(SkyType::Count) == sizeof(SkyTypeName) / sizeof(char *),
	"Sky type and names mismatch.");

CD_FORCEINLINE const char *GetSkyTypeName(SkyType type)
{
	return SkyTypeName[static_cast<size_t>(type)];
}

class SkyComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("SkyComponent");
		return className;
	}

	SkyComponent() = default;
	SkyComponent(const SkyComponent&) = default;
	SkyComponent& operator=(const SkyComponent&) = default;
	SkyComponent(SkyComponent&&) = default;
	SkyComponent& operator=(SkyComponent&&) = default;
	~SkyComponent() = default;

	void SetSkyType(SkyType type);
	SkyType GetSkyType() const { return m_type; }

private:
	SkyType m_type = SkyType::SkyBox;
};

}
