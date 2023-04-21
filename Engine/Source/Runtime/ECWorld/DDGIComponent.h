#pragma once

#include "Core/StringCrc.h"
#include "Math/Matrix.hpp"

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

	void SetClassificationRawData(const std::string& path);
	const uint8_t* GetClassificationRawData() const { return m_classificationRawData.data(); }
	uint32_t GetClassificationSize() const { return static_cast<uint32_t>(m_classificationRawData.size()); }

	void SetDistanceRawData(const std::string& path);
	const uint8_t* GetDistanceRawData() const { return m_distanceRawData.data(); }
	uint32_t GetDistanceSize() const { return static_cast<uint32_t>(m_distanceRawData.size()); }

	void SetIrradianceRawData(const std::string& path);
	const uint8_t* GetIrradianceRawData() const { return m_irradianceRawData.data(); }
	uint32_t GetIrradianceSize() const { return static_cast<uint32_t>(m_irradianceRawData.size()); }

	void SetRelocationRawData(const std::string& path);
	const uint8_t* GetRelocationRawData() const { return m_relocationRawData.data(); }
	uint32_t GetRelocationSize() const { return static_cast<uint32_t>(m_relocationRawData.size()); }

	void SetDimension(const cd::Vec3f &dimension) { m_dimension = dimension; }
	const cd::Vec3f &GetDimension() const { return m_dimension; }

	void SetSpacing(const cd::Vec3f &spacing) { m_spacing = spacing; }
	const cd::Vec3f &GetSpacing() const { return m_spacing; }

	void SetAmbientMultiplier(float multiplier) { m_ambientMultiplier = multiplier; }
	const float GetAmbientMultiplier() const { return m_ambientMultiplier; }

	void SetViewBias(float bias) { m_viewBias = bias; }
	const float GetViewBias() const { return m_viewBias; }
	
	void SetNormalBias(float bias) { m_normalBias = bias; }
	const float GetNormalBias() const { return m_normalBias; }

private:
	std::vector<uint8_t> m_classificationRawData;
	std::vector<uint8_t> m_distanceRawData;
	std::vector<uint8_t> m_irradianceRawData;
	std::vector<uint8_t> m_relocationRawData;

	cd::Vec3f m_dimension;
	cd::Vec3f m_spacing;

	float m_ambientMultiplier;
	float m_viewBias;
	float m_normalBias;
};

}
