#pragma once

#include "Math/VectorDerived.hpp"

namespace engine
{

class TransformComponent
{
public:
	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent& operator=(const TransformComponent&) = default;
	TransformComponent(TransformComponent&&) = default;
	TransformComponent& operator=(TransformComponent&&) = default;
	~TransformComponent() = default;

	cd::Vec3f GetTranslation() const { return m_translation; }
	cd::Vec3f GetRotation() const { return m_rotation; }
	cd::Vec3f GetScale() const { return m_scale; }

private:
	cd::Vec3f m_translation;
	cd::Vec3f m_rotation;
	cd::Vec3f m_scale;
};

}