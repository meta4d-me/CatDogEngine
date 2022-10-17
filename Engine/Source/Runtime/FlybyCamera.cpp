#include "FlybyCamera.h"

#include <memory>

namespace engine
{

FlybyCamera::FlybyCamera() 
	: Camera()
	, m_position(0.0f, 0.0f, 0.0f)
	, m_forwardDirection(0.0f, 0.0f, 1.0f)
	, m_upDirection(0.0f, 1.0f, 0.0f)
{
	memset(m_viewMatrix, 0, 16 * sizeof(float));
}

FlybyCamera::FlybyCamera(const bx::Vec3& position)
	: Camera()
	, m_position(position)
	, m_forwardDirection(0.0f, 0.0f, 1.0f)
	, m_upDirection(0.0f, 1.0f, 0.0f)
{
	memset(m_viewMatrix, 0, 16 * sizeof(float));
}

FlybyCamera::FlybyCamera(const bx::Vec3& position, const bx::Vec3& forward, const bx::Vec3& up)
	: Camera()
	, m_position(position)
	, m_forwardDirection(bx::normalize(forward))
	, m_upDirection(bx::normalize(up))
{
	memset(m_viewMatrix, 0, 16 * sizeof(float));
}

void FlybyCamera::MoveForward(const float amount)
{
	m_position = bx::mad(m_forwardDirection, amount, m_position);
}

void FlybyCamera::MoveBackward(const float amount)
{
	MoveForward(-amount);
}

void FlybyCamera::MoveLeft(const float amount)
{
	const bx::Vec3 leftAxis = bx::normalize(bx::cross(m_forwardDirection, m_upDirection));
	m_position = bx::mad(leftAxis, amount, m_position);
}

void FlybyCamera::MoveRight(const float amount)
{
	MoveLeft(-amount);
}

void FlybyCamera::MoveUp(const float amount)
{
	m_position = bx::mad(m_upDirection, amount, m_position);
}

void FlybyCamera::MoveDown(const float amount)
{
	MoveUp(-amount);
}

void FlybyCamera::Rotate(const bx::Vec3& axis, const float angleDegrees)
{
	const bx::Quaternion rotation = bx::fromAxisAngle(axis, bx::toRad(angleDegrees));
	m_forwardDirection = bx::normalize(bx::mul(m_forwardDirection, rotation));
	m_upDirection = bx::normalize(bx::mul(m_upDirection, rotation));
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

void FlybyCamera::YawLocal(const float angleDegrees)
{
	Rotate(m_upDirection, angleDegrees);
}

void FlybyCamera::PitchLocal(const float angleDegrees)
{
	const bx::Vec3 leftAxis = bx::normalize(bx::cross(m_forwardDirection, m_upDirection));
	Rotate(leftAxis, angleDegrees);
}

void FlybyCamera::RollLocal(const float angleDegrees)
{
	Rotate(m_forwardDirection, angleDegrees);
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
	bx::mtxLookAt(m_viewMatrix, m_position, bx::add(m_position, m_forwardDirection), m_upDirection);

	m_dirty = false;
}

}	// namespace engine