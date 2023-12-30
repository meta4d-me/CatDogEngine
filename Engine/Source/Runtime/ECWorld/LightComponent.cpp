#include "LightComponent.h"

namespace engine
{

cd::Vec2f LightComponent::GetInnerAndOuter() const
{
	// outerCos = cos(outerAngle)
	// -> outerAngle = RadToDeg(std::acos(outerCos))
	// scale = 1 / (cos(innerAngle) - outerCos)
	// -> RadToDeg(std::acos(1.0f / scale + outerCos))

	float outerCos = -m_lightUniformData.lightAngleOffeset / m_lightUniformData.lightAngleScale;

	return cd::Vec2f(cd::Math::RadianToDegree(std::acos(static_cast<float>(std::clamp(1.0f / m_lightUniformData.lightAngleScale + outerCos, 0.0f, 1.0f)))), cd::Math::RadianToDegree(std::acos(outerCos)));
}

void LightComponent::SetInnerAndOuter(float inner, float outer)
{
	inner = inner > outer ? outer : inner;

	float outerCos = std::cos(cd::Math::DegreeToRadian(outer));
	float scale = 1.0f / std::max(std::cos(cd::Math::DegreeToRadian(inner)) - outerCos, 0.001f);

	m_lightUniformData.lightAngleScale = scale;
	m_lightUniformData.lightAngleOffeset = -outerCos * scale;
}

bool LightComponent::IsShadowMapFBsValid()
{
	// Include empty FBs
	if ((cd::LightType::Spot == GetType() && m_shadowMapFBs.size() != 1U) ||
		(cd::LightType::Point == GetType() && m_shadowMapFBs.size() != 6U) ||
		(cd::LightType::Directional == GetType() && m_shadowMapFBs.size() != GetCascadeNum()))
	{
		return false;
	}

	for (const auto& fb : m_shadowMapFBs)
	{
		if (!bgfx::isValid(fb)) return false;
	}
	return true;
}

}
