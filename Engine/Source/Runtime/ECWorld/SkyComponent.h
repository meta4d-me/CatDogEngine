#pragma once

#include "Core/Types.h"
#include "Rendering/SkyType.h"

#include <string>

namespace engine
{

class SkyComponent final
{
public:
	static constexpr const char* DefaultIrradainceTexturePath = "Textures/skybox/defaultSkybox_irr.dds";
	static constexpr const char* DefaultRadianceTexturePath = "Textures/skybox/defaultSkybox_rad.dds";
	static constexpr const char* DefaultPureGrayTexturePath = "Textures/skybox/PureGray.dds";

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
	SkyType& GetSkyType() { return m_type; }
	const SkyType& GetSkyType() const { return m_type; }

	void SetSkyboxStrength(float strength) { m_skyboxStrength = strength; }
	float& GetSkyboxStrength() { return m_skyboxStrength; }
	float GetSkyboxStrength() const { return m_skyboxStrength; }

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

	void SetHeightOffset(float height) { m_heightOffset = height; }
	float& GetHeightOffset() { return m_heightOffset; }
	const float& GetHeightOffset() const { return m_heightOffset; }

	void SetShadowLength(float length) { m_shadowLength = length; }
	float& GetShadowLength() { return m_shadowLength; }
	const float& GetShadowLength() const { return m_shadowLength; }

	void SetSunDirection(cd::Direction dir);
	cd::Direction& GetSunDirection() { return m_sunDirection; }
	const cd::Direction& GetSunDirection() const { return m_sunDirection; }

	void SetIrradianceTexturePath(std::string path);
	std::string& GetIrradianceTexturePath() { return m_irradianceTexturePath; }
	const std::string& GetIrradianceTexturePath() const { return m_irradianceTexturePath; }

	void SetRadianceTexturePath(std::string path);
	std::string& GetRadianceTexturePath() { return m_radianceTexturePath; }
	const std::string& GetRadianceTexturePath() const { return m_radianceTexturePath; }

	void SetPureGrayTexturePath(std::string path);
	std::string& GetPureGrayTexturePath() { return m_pureGrayTexturePath; }
	const std::string& GetPureGrayTexturePath() const { return m_pureGrayTexturePath; }

private:
	SkyType m_type = SkyType::SkyBox;
	float m_skyboxStrength = 0.5f;
	bool m_isAtmophericScatteringEnable = false;
	
	StringCrc m_ATMTransmittanceCrc;
	StringCrc m_ATMIrradianceCrc;
	StringCrc m_ATMScatteringCrc;

	// Unit : km
	float m_heightOffset = 1.0f;
	float m_shadowLength = 0.1f;
	cd::Direction m_sunDirection = cd::Direction(0.0f, -1.0f, 0.0f);

	std::string m_irradianceTexturePath = DefaultIrradainceTexturePath;
	std::string m_radianceTexturePath = DefaultRadianceTexturePath;
	std::string m_pureGrayTexturePath = DefaultPureGrayTexturePath;
};

}