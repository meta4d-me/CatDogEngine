#pragma once

#include "Core/StringCrc.h"
#include "Rendering/LightUniforms.h"
#include "Scene/LightType.h"

namespace engine
{

class LightComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("LightComponent");
		return className;
	}

public:
	LightComponent() = default;
	LightComponent(const LightComponent&) = default;
	LightComponent& operator=(const LightComponent&) = default;
	LightComponent(LightComponent&&) = default;
	LightComponent& operator=(LightComponent&&) = default;
	~LightComponent() = default;

	void SetType(cd::LightType type) { m_lightUniformData.type = static_cast<float>(type); }
	cd::LightType GetType() const { return static_cast<cd::LightType>(m_lightUniformData.type); }

	void SetColor(cd::Vec3f color) { m_lightUniformData.color = cd::MoveTemp(color); }
	cd::Vec3f& GetColor() { return m_lightUniformData.color; }
	const cd::Vec3f& GetColor() const { return m_lightUniformData.color; }

	void SetIntensity(float intensity) { m_lightUniformData.intensity = intensity; }
	float& GetIntensity() { return m_lightUniformData.intensity; }
	float GetIntensity() const { return m_lightUniformData.intensity; }

	void SetRange(float range) { m_lightUniformData.range = range; }
	float& GetRange() { return m_lightUniformData.range; }
	float GetRange() const { return m_lightUniformData.range; }

	void SetRadius(float radius) { m_lightUniformData.radius = radius; }
	float& GetRadius() { return m_lightUniformData.radius; }
	float GetRadius() const { return m_lightUniformData.radius; }

	void SetWidth(float width) { m_lightUniformData.width = width; }
	float& GetWidth() { return m_lightUniformData.width; }
	float GetWidth() const { return m_lightUniformData.width; }

	void SetHeight(float height) { m_lightUniformData.height = height; }
	float& GetHeight() { return m_lightUniformData.height; }
	float GetHeight() const { return m_lightUniformData.height; }

	void RecalculateScaleAndOffset();

	void SetInnerDegree(float inner);
	float &GetInnerDegree() { return m_spotInnerDegree; }
	float GetInnerDegree() const { return m_spotInnerDegree; }

	void SetOuterDegree(float outer);
	float &GetOuterDegree() { return m_spotOuterDegree; }
	float GetOuterDegree() const { return m_spotOuterDegree; }

	// It is recommended to access the inside and outside angle of the spotlight through the functions of the Degree series.
	void SetAngleScale(float angleScale) { m_lightUniformData.lightAngleScale = angleScale; }
	float& GetAngleScale() { return m_lightUniformData.lightAngleScale; }
	float GetAngleScale() const { return m_lightUniformData.lightAngleScale; }

	// It is recommended to access the inside and outside angle of the spotlight through the functions of the Degree series.
	void SetAngleOffset(float angleOffset) { m_lightUniformData.lightAngleOffeset = angleOffset; }
	float& GetAngleOffset() { return m_lightUniformData.lightAngleOffeset; }
	float GetAngleOffset() const { return m_lightUniformData.lightAngleOffeset; }

	void SetPosition(cd::Point position) { m_lightUniformData.position = cd::MoveTemp(position); }
	cd::Point& GetPosition() { return m_lightUniformData.position; }
	const cd::Point& GetPosition() const { return m_lightUniformData.position; }

	void SetDirection(cd::Direction direction) { m_lightUniformData.direction = cd::MoveTemp(direction); }
	cd::Direction& GetDirection() { return m_lightUniformData.direction; }
	const cd::Direction& GetDirection() const { return m_lightUniformData.direction; }

	void SetUp(cd::Direction up) { m_lightUniformData.up = cd::MoveTemp(up); }
	cd::Direction& GetUp() { return m_lightUniformData.up; }
	const cd::Direction& GetUp() const { return m_lightUniformData.up; }

private:
	U_Light m_lightUniformData;

	// Just for UI.
	float m_spotInnerDegree = -1.0f;
	float m_spotOuterDegree = -1.0f;
};

}