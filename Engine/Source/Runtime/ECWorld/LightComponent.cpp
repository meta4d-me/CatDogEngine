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
	return cd::Vec2f(cd::Math::RadianToDegree(std::acos(1.0f / std::max(m_lightUniformData.lightAngleScale, 0.001f) + outerCos)), cd::Math::RadianToDegree(std::acos(outerCos)));
}

void LightComponent::SetInnerAndOuter(float inner, float outer)
{
	float outerCos = std::cos(cd::Math::DegreeToRadian(outer));
	float scale = 1.0f / std::max(std::cos(cd::Math::DegreeToRadian(inner)) - outerCos, 0.001f);

	m_lightUniformData.lightAngleScale = scale;
	m_lightUniformData.lightAngleOffeset = -outerCos * scale;
}

}
