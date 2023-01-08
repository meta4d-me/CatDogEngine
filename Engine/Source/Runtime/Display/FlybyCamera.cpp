#include "FlybyCamera.h"

#include <memory>

namespace engine
{

void FlybyCamera::MoveForward(const float amount)
{
	m_position += m_forwardDirection * amount;
	Dirty();
}

void FlybyCamera::MoveBackward(const float amount)
{
	MoveForward(-amount);
}

void FlybyCamera::MoveLeft(const float amount)
{
	m_position += m_forwardDirection.Cross(m_upDirection).Normalize() * amount;
	Dirty();
}

void FlybyCamera::MoveRight(const float amount)
{
	MoveLeft(-amount);
}

void FlybyCamera::MoveUp(const float amount)
{
	m_position += m_upDirection * amount;
	Dirty();
}

void FlybyCamera::MoveDown(const float amount)
{
	MoveUp(-amount);
}

void FlybyCamera::Rotate(const cd::Vec3f& axis, const float angleDegrees)
{
	//const bx::Quaternion rotation = bx::fromAxisAngle(axis, bx::toRad(angleDegrees));
	//m_forwardDirection = bx::normalize(bx::mul(m_forwardDirection, rotation));
	//m_upDirection = bx::normalize(bx::mul(m_upDirection, rotation));
	Dirty();
}

void FlybyCamera::Rotate(const float x, const float y, const float z, const float angleDegrees)
{
	Rotate(cd::Vec3f(x, y, z), angleDegrees);
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
	Rotate(m_forwardDirection.Cross(m_upDirection).Normalize(), angleDegrees);
}

void FlybyCamera::RollLocal(const float angleDegrees)
{
	Rotate(m_forwardDirection, angleDegrees);
}

}	// namespace engine