#pragma once

#include "Core/StringCrc.h"
#include "Math/Transform.hpp"

namespace engine
{

class TransformComponent final
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

	const cd::Transform& GetTransform() const { return m_transform; }
	cd::Transform& GetTransform() { return m_transform; }
	void SetTransform(cd::Transform transform) { m_transform = cd::MoveTemp(transform); m_isMatrixDirty = true;  }

	const cd::Matrix4x4& GetWorldMatrix() const { return m_localToWorldMatrix; }

	void Dirty() const { m_isMatrixDirty = true; }

	void Reset();
	void Build();

	//Camera's features from transform
	//cd::Vec3f GetEye() { return this->GetTransform().GetTranslation(); }
	//void SetEye(cd::Vec3f eye) { this->GetTransform().GetTranslation() = cd::MoveTemp(eye); }

	//cd::Vec3f GetLookAt() { return this->GetTransform().GetRotation().ToMatrix3x3() * cd::Vec3f(0, 0, 1); }
	//void SetLookAt(cd::Vec3f lookAt) { this->GetTransform().SetRotation(cd::Quaternion::FromAxisAngle(GetLookAt().Cross(lookAt), std::acos(GetLookAt().Dot(lookAt)))); }

	//cd::Vec3f GetUp() { return this->GetTransform().GetRotation().ToMatrix3x3() * cd::Vec3f(0, 1, 0); }
	//void SetUp(cd::Vec3f up) { this->GetTransform().SetRotation(cd::Quaternion::FromAxisAngle(GetLookAt().Cross(up), std::acos(GetLookAt().Dot(up)))); }

	//cd::Vec3f GetCross() { return this->GetTransform().GetRotation().ToMatrix3x3() * cd::Vec3f(1, 0, 0); }
	//void SetCross(cd::Vec3f cross) { this->GetTransform().SetRotation(cd::Quaternion::FromAxisAngle(GetLookAt().Cross(cross), std::acos(GetLookAt().Dot(cross)))); }
#ifdef EDITOR_MODE
	static bool DoUseUniformScale() { return m_doUseUniformScale; }
	static void SetUseUniformScale(bool use) { m_doUseUniformScale = use; }
#endif

private:
	// Input
	cd::Transform m_transform;

	// Status
	mutable bool m_isMatrixDirty;

	// Output
	cd::Matrix4x4 m_localToWorldMatrix;

#ifdef EDITOR_MODE
	static bool m_doUseUniformScale;
#endif
};

}