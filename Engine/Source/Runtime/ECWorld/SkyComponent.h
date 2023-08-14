#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"

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
	SkyType& GetSkyType() { return m_type; }
	const SkyType& GetSkyType() const { return m_type; }

	void SetAtmophericScatteringEnable(bool state) { m_isAtmophericScatteringEnable = state; }
	bool& GetAtmophericScatteringEnable() { return m_isAtmophericScatteringEnable; }
	const bool& GetAtmophericScatteringEnable() const { return m_isAtmophericScatteringEnable; }

	void SetATMTransmittanceCrc(StringCrc crc) { m_ATMTransmittanceCrc = crc; }
	StringCrc& GetATMTransmittanceCrc() { return m_ATMTransmittanceCrc; }
	const StringCrc& GetATMTransmittanceCrc() const { return m_ATMTransmittanceCrc; }

	void SetATMIrradianceCrc(StringCrc crc) { m_ATMIrradianceCrc = crc; }
	StringCrc& GetATMIrradianceCrc() { return m_ATMIrradianceCrc; }
	const StringCrc& GetATMIrradianceCrc() const { return m_ATMIrradianceCrc; }

	void SetATMScatteringCrc(StringCrc crc) { m_ATMScatteringCrc = crc; }
	StringCrc& GetATMScatteringCrc() { return m_ATMScatteringCrc; }
	const StringCrc& GetATMScatteringCrc() const { return m_ATMScatteringCrc; }

	void SetSunDirection(cd::Direction dir);
	cd::Direction& GetSunDirection() { return m_sunDirection; }
	const cd::Direction& GetSunDirection() const { return m_sunDirection; }

	void SetHeightOffset(float height) { m_heightOffset = height; }
	float& GetHeightOffset() { return m_heightOffset; }
	const float& GetHeightOffset() const { return m_heightOffset; }

	void SetIrradianceTexturePath(std::string path);
	std::string& GetIrradianceTexturePath() { return m_irradianceTexturePath; }
	const std::string& GetIrradianceTexturePath() const { return m_irradianceTexturePath; }

	void SetRadianceTexturePath(std::string path);
	std::string& GetRadianceTexturePath() { return m_radianceTexturePath; }
	const std::string& GetRadianceTexturePath() const { return m_radianceTexturePath; }

private:
	SkyType m_type = SkyType::SkyBox;
	bool m_isAtmophericScatteringEnable = false;
	
	StringCrc m_ATMTransmittanceCrc;
	StringCrc m_ATMIrradianceCrc;
	StringCrc m_ATMScatteringCrc;
	cd::Direction m_sunDirection = cd::Direction(0.0f, -1.0f, 0.0f);
	float m_heightOffset = 1.0f; // Unit : km

	std::string m_irradianceTexturePath = DefaultIrradainceTexturePath;
	std::string m_radianceTexturePath = DefaultRadianceTexturePath;
};

}