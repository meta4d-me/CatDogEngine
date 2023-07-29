#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"

#include <string>

namespace engine
{

enum class SkyType
{
	None = 0,
	SkyBox,
	AtmosphericScattering,

	Count,
};

constexpr const char* SkyTypeName[] =
{
	"None",
	"Skybox",
	"Atmospheric Scattering",
};

static_assert(static_cast<int>(SkyType::Count) == sizeof(SkyTypeName) / sizeof(char*),
	"Sky type and name mismatch.");

CD_FORCEINLINE const char* GetSkyTypeName(SkyType type)
{
	return SkyTypeName[static_cast<size_t>(type)];
}

class SkyComponent final
{
public:
	static constexpr const char* DefaultIrradainceTexturePath = "Textures/skybox/defaultSkybox_irr.dds";
	static constexpr const char* DefaultRadianceTexturePath = "Textures/skybox/defaultSkybox_rad.dds";
	static constexpr const char* PureGrayTexturePath = "Textures/skybox/PureGray.dds";

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

	void SetSkyType(SkyType crtType);
	const SkyType& GetSkyType() const { return m_type; }
	SkyType& GetSkyType() { return m_type; }

	void SetIrradianceTexturePath(std::string path);
	std::string& GetIrradianceTexturePath() { return m_irradianceTexturePath; }
	const std::string& GetIrradianceTexturePath() const { return m_irradianceTexturePath; }

	void SetRadianceTexturePath(std::string path);
	std::string& GetRadianceTexturePath() { return m_radianceTexturePath; }
	const std::string& GetRadianceTexturePath() const { return m_radianceTexturePath; }

private:
	SkyType m_type = SkyType::SkyBox;
	std::string m_irradianceTexturePath = DefaultIrradainceTexturePath;
	std::string m_radianceTexturePath = DefaultRadianceTexturePath;
};

}