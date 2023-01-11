#pragma once

#include "Core/StringCrc.h"
#include "Math/Matrix.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Vector.hpp"

namespace engine
{

class TransformComponent
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("TransformComponent");
		return className;
	}

public:
	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent& operator=(const TransformComponent&) = default;
	TransformComponent(TransformComponent&&) = default;
	TransformComponent& operator=(TransformComponent&&) = default;
	~TransformComponent() = default;

	const cd::Vec3f& GetTranslation() const { return m_translation; }
	cd::Vec3f& GetTranslation() { return m_translation; }
	void SetTranslation(cd::Vec3f translation) { m_translation = cd::MoveTemp(translation); }

	const cd::Quaternion& GetRotation() const { return m_rotation; }
	cd::Quaternion& GetRotation() { return m_rotation; }
	void SetRotation(cd::Quaternion rotation) { m_rotation = cd::MoveTemp(rotation); }

	const cd::Vec3f& GetScale() const { return m_scale; }
	cd::Vec3f& GetScale() { return m_scale; }
	void SetScale(cd::Vec3f scale) { m_scale = cd::MoveTemp(scale); }

	const cd::Matrix4x4& GetTransformation() const { return m_transformation; }
	cd::Matrix4x4& GetTransformation() { return m_transformation; }

	void Build();

private:
	// Input
	cd::Vec3f m_translation;
	cd::Quaternion m_rotation;
	cd::Vec3f m_scale;

	// Output
	cd::Matrix4x4 m_transformation;
};

}