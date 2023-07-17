#pragma once

#include "Core/StringCrc.h"
#include "Math/Matrix.hpp"

#include <memory>
#include <vector>

namespace engine
{

class DDGIComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("DDGIComponent");
		return className;
	}

	void ResetTextureRawData();

	void SetClassificationRawData(const std::string& path);
	void SetClassificationRawData(const std::shared_ptr<std::vector<uint8_t>>& classification);
	const uint8_t* GetClassificationRawData() const { return m_classificationRawData.data(); }
	uint32_t GetClassificationSize() const { return static_cast<uint32_t>(m_classificationRawData.size()); }

	void SetDistanceRawData(const std::string& path);
	void SetDistanceRawData(const std::shared_ptr<std::vector<uint8_t>>& distance);
	const uint8_t* GetDistanceRawData() const { return m_distanceRawData.data(); }
	uint32_t GetDistanceSize() const { return static_cast<uint32_t>(m_distanceRawData.size()); }

	void SetIrradianceRawData(const std::string& path);
	void SetIrradianceRawData(const std::shared_ptr<std::vector<uint8_t>>& irrdiance);
	const uint8_t* GetIrradianceRawData() const { return m_irradianceRawData.data(); }
	uint32_t GetIrradianceSize() const { return static_cast<uint32_t>(m_irradianceRawData.size()); }

	void SetRelocationRawData(const std::string& path);
	void SetRelocationRawData(const std::shared_ptr<std::vector<uint8_t>>& relocation);
	const uint8_t* GetRelocationRawData() const { return m_relocationRawData.data(); }
	uint32_t GetRelocationSize() const { return static_cast<uint32_t>(m_relocationRawData.size()); }

	void SetVolumeOrigin(const cd::Vec3f& origin) { m_volumeOrigin = origin; }
	const cd::Vec3f& GetVolumeOrigin() const { return m_volumeOrigin; }
	cd::Vec3f& GetVolumeOrigin() { return m_volumeOrigin; }

	void SetProbeSpacing(const cd::Vec3f& spacing) { m_probeSpacing = spacing; }
	const cd::Vec3f& GetProbeSpacing() const { return m_probeSpacing; }
	cd::Vec3f& GetProbeSpacing() { return m_probeSpacing; }

	void SetProbeCount(const cd::Vec3f& count) { m_probeCount = count; }
	const cd::Vec3f& GetProbeCount() const { return m_probeCount; }
	cd::Vec3f& GetProbeCount() { return m_probeCount; }

	void SetAmbientMultiplier(float multiplier) { m_ambientMultiplier = multiplier; }
	const float& GetAmbientMultiplier() const { return m_ambientMultiplier; }
	float& GetAmbientMultiplier() { return m_ambientMultiplier; }

	void SetViewBias(float bias) { m_viewBias = bias; }
	const float& GetViewBias() const { return m_viewBias; }
	float& GetViewBias() { return m_viewBias; }
	
	void SetNormalBias(float bias) { m_normalBias = bias; }
	const float& GetNormalBias() const { return m_normalBias; }
	float& GetNormalBias() { return m_normalBias; }

private:
	std::vector<uint8_t> m_classificationRawData;
	std::vector<uint8_t> m_distanceRawData;
	std::vector<uint8_t> m_irradianceRawData;
	std::vector<uint8_t> m_relocationRawData;

	cd::Vec3f m_volumeOrigin;
	cd::Vec3f m_probeSpacing;
	cd::Vec3f m_probeCount;

	float m_ambientMultiplier;
	float m_viewBias;
	float m_normalBias;
};

}
