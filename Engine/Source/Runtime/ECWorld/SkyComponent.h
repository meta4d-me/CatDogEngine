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
	"Sky type and name mismatch.");

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
	const SkyType& GetSkyType() const { return m_type; }
	SkyType& GetSkyType() { return m_type; }

	void SetIrradianceTexturePath(std::string path);
	std::string& GetIrradianceTexturePath() { return m_irradianceTexturePath; }
	const std::string& GetIrradianceTexturePath() const { return m_irradianceTexturePath; }

	void SetRadianceTexturePath(std::string path);
	std::string &GetRadianceTexturePath() { return m_radianceTexturePath; }
	const std::string &GetRadianceTexturePath() const { return m_radianceTexturePath; }

private:
	SkyType m_type = SkyType::SkyBox;
	std::string m_irradianceTexturePath = "Textures/skybox/defaultSkybox_irr.dds";
	std::string m_radianceTexturePath = "Textures/skybox/defaultSkybox_rad.dds";
};

}
