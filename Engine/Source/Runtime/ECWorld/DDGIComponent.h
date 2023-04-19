#pragma once

#include "Core/StringCrc.h"
#include "Math/Matrix.hpp"

#include <span>
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

	void SetClassificationTexturePath(const std::string &path) { m_classificationTexturePath = path; }
	const std::string &GetClassificationTexturePath() const { return m_classificationTexturePath; }

	void SetDistanceTexturePath(const std::string &path) { m_distanceTexturePath = path; }
	const std::string &GetDistanceTexturePath() const { return m_distanceTexturePath; }

	void SetIrradianceTexturePath(const std::string &path) { m_irradianceTexturePath = path; }
	const std::string &GetIrradianceTexturePath() const { return m_irradianceTexturePath; }

	void SetRelocationTexturePath(const std::string &path) { m_relocationTexturePath = path; }
	const std::string &GetRelocationTexturePath() const { return m_relocationTexturePath; }

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
	std::string m_classificationTexturePath;
	std::string m_distanceTexturePath;
	std::string m_irradianceTexturePath;
	std::string m_relocationTexturePath;

	cd::Vec3f m_dimension;
	cd::Vec3f m_spacing;

	float m_ambientMultiplier;
	float m_viewBias;
	float m_normalBias;
};

}
