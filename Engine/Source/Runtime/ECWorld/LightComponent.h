#pragma once

#include "Core/StringCrc.h"
#include "Scene/Light.h"

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

	void SetType(cd::LightType type) { m_lightType = type; }
	cd::LightType GetType() const { return m_lightType; }

	void SetIntensity(float intensity) { m_intensity = intensity; }
	float GetIntensity() const { return m_intensity; }

	void SetRange(float range) { m_range = range; }
	float GetRange() const { return m_range; }

	void SetRadius(float radius) { m_radius = radius; }
	float GetRadius() const { return m_radius; }

	void SetWidth(float width) { m_width = width; }
	float GetWidth() const { return m_width; }

	void SetHeight(float height) { m_height = height; }
	float GetHeight() const { return m_height; }

	void SetAngleScale(float angleScale) { m_angleScale = angleScale; }
	float GetAngleScale() const { return m_angleScale; }

	void SetAngleOffset(float angleOffset) { m_angleOffset = angleOffset; }
	float GetAngleOffset() const { return m_angleOffset; }

	void SetColor(cd::Vec3f color) { m_color = cd::MoveTemp(color); }
	const cd::Vec3f& GetColor() const { return m_color; }

	void SetPosition(cd::Point position) { m_position = cd::MoveTemp(position); }
	const cd::Point& GetPosition() const { return m_position; }

	void SetDirection(cd::Direction direction) { m_direction = cd::MoveTemp(direction); }
	const cd::Direction& GetDirection() const { return m_direction; }

	void SetUp(cd::Direction up) { m_up = cd::MoveTemp(up); }
	const cd::Direction& GetUp() const { return m_up; }

private:
	cd::LightType m_lightType;

	float m_intensity;
	float m_range;
	float m_radius;
	float m_width;
	float m_height;
	float m_angleScale;
	float m_angleOffset;

	cd::Point m_position;
	cd::Vec3f m_color;
	cd::Direction m_direction;
	cd::Direction m_up;
};

}