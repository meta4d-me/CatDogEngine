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

}

void LightComponent::RecalculateScaleAndOffset()
{
	float outerCos = std::cos(DegToRad(m_spotOuterDegree));
	float scale = 1.0f / std::max(std::cos(DegToRad(m_spotInnerDegree)) - outerCos, 0.001f);

	m_lightUniformData.lightAngleScale = scale;
	m_lightUniformData.lightAngleOffeset = -outerCos * scale;
}

void LightComponent::SetInnerDegree(float inner) {
	m_spotInnerDegree = inner;
	if(m_spotOuterDegree < 0.0f)
	{
		// A little trick to avoid uninitialized angle.
		m_spotOuterDegree = 1.25f * m_spotInnerDegree;
	}
	RecalculateScaleAndOffset();
}

void LightComponent::SetOuterDegree(float outer) {
	m_spotOuterDegree = outer;
	if(m_spotInnerDegree < 0.0f)
	{
		// A little trick to avoid uninitialized angle.
		m_spotInnerDegree = 0.75f * m_spotOuterDegree;
	}
	RecalculateScaleAndOffset();
}

}
