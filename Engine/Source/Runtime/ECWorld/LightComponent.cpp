#include "LightComponent.h"

namespace engine
{

namespace
{

CD_FORCEINLINE float DegToRad(float deg)
{
	// pi / 180
	return deg * 0.01745329251994329576923690768489f;
}

CD_FORCEINLINE float RadToDeg(float rad)
{
	// 180 / pi
	return rad * 57.295779513082320876798154814105f;
}

}

cd::Vec2f LightComponent::GetInnerAndOuter() const
{
	// outerCos = cos(outerAngle)
	// -> outerAngle = RadToDeg(std::acos(outerCos))
	// scale = 1 / (cos(innerAngle) - outerCos)
	// -> RadToDeg(std::acos(1.0f / scale + outerCos))

	float outerCos = -m_lightUniformData.lightAngleOffeset / m_lightUniformData.lightAngleScale;
	return cd::Vec2f(RadToDeg(std::acos(1.0f / std::max(m_lightUniformData.lightAngleScale, 0.001f) + outerCos)), RadToDeg(std::acos(outerCos)));
}

void LightComponent::SetInnerAndOuter(float inner, float outer)
{
	float outerCos = std::cos(DegToRad(outer));
	float scale = 1.0f / std::max(std::cos(DegToRad(inner)) - outerCos, 0.001f);

	m_lightUniformData.lightAngleScale = scale;
	m_lightUniformData.lightAngleOffeset = -outerCos * scale;
}

}
