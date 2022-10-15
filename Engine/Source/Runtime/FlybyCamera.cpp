#include "FlybyCamera.h"

#include <memory>

namespace engine
{

FlybyCamera::FlybyCamera() 
	: Camera()
	, m_position(0.0f, 0.0f, 0.0f)
	, m_orientation(bx::init::Identity)
{
	memset(m_viewMatrix, 0, 16 * sizeof(float));
}

FlybyCamera::FlybyCamera(const bx::Vec3& position)
	: Camera()
	, m_position(position)
	, m_orientation(bx::init::Identity)
{
	memset(m_viewMatrix, 0, 16 * sizeof(float));
}

FlybyCamera::FlybyCamera(const bx::Vec3& position, const bx::Quaternion& orientation)
	: Camera()
	, m_position(position)
	, m_orientation(orientation)
{
	memset(m_viewMatrix, 0, 16 * sizeof(float));
}

void FlybyCamera::Translate(const bx::Vec3& v)
{
	m_position = bx::add(m_position, bx::mul(v, m_orientation));
	Dirty();
}

void FlybyCamera::Translate(const float x, const float y, const float z)
{
	Translate(bx::Vec3(x, y, z));
}

void FlybyCamera::Rotate(const bx::Vec3& axis, const float angleDegrees)
{
	m_orientation = bx::normalize(bx::fromAxisAngle(axis, angleDegrees));
	Dirty();
}

void FlybyCamera::Rotate(const float x, const float y, const float z, const float angleDegrees)
{
	Rotate(bx::Vec3(x, y, z), angleDegrees);
}

void FlybyCamera::Yaw(const float angleDegrees)
{
	Rotate(0.0f, 1.0f, 0.0f, angleDegrees);
}

void FlybyCamera::Pitch(const float angleDegrees)
{
	Rotate(1.0f, 0.0f, 0.0f, angleDegrees);
}

void FlybyCamera::Roll(const float angleDegrees)
{
	Rotate(0.0f, 0.0f, 1.0f, angleDegrees);
}

void FlybyCamera::RotateLocal(const bx::Vec3& axis, const float angleDegrees)
{
	const bx::Vec3 rotatedLocalAxis = bx::mul(axis, m_orientation);
	m_orientation = bx::normalize(bx::fromAxisAngle(rotatedLocalAxis, angleDegrees));
	Dirty();
}

void FlybyCamera::RotateLocal(const float x, const float y, const float z, const float angleDegrees)
{
	RotateLocal(bx::Vec3(x, y, z), angleDegrees);
}

void FlybyCamera::YawLocal(const float angleDegrees)
{
	RotateLocal(0.0f, 1.0f, 0.0f, angleDegrees);
}

void FlybyCamera::PitchLocal(const float angleDegrees)
{
	RotateLocal(1.0f, 0.0f, 0.0f, angleDegrees);
}

void FlybyCamera::RollLocal(const float angleDegrees)
{
	RotateLocal(0.0f, 0.0f, 1.0f, angleDegrees);
}

const float* FlybyCamera::GetViewMatrix() const {
	return m_viewMatrix;
}

void FlybyCamera::Update()
{
	if (!m_dirty) return;

	// Update NDC
	Camera::Update();

	// Update view matrix
	bx::mtxFromQuaternion(m_viewMatrix, m_orientation, m_position);

	m_dirty = false;
}

}	// namespace engine