#include "FlybyCamera.h"

#include "Math/Quaternion.hpp"

#include <memory>

namespace engine
{

void FlybyCamera::MoveForward(float amount)
{
	m_position += m_forwardDirection * amount;
	Dirty();
}

void FlybyCamera::MoveBackward(float amount)
{
	MoveForward(-amount);
}

void FlybyCamera::MoveLeft(float amount)
{
	m_position += m_forwardDirection.Cross(m_upDirection).Normalize() * amount;
	Dirty();
}

void FlybyCamera::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void FlybyCamera::MoveUp(float amount)
{
	m_position += m_upDirection * amount;
	Dirty();
}

void FlybyCamera::MoveDown(float amount)
{
	MoveUp(-amount);
}

void FlybyCamera::Rotate(const cd::Vec3f& axis, float angleDegrees)
{
	cd::Quaternion rotation = cd::Quaternion::FromAxisAngle(axis, cd::DegreeToRadian<float>(angleDegrees));
	m_forwardDirection = (rotation * m_forwardDirection).Normalize();
	m_upDirection = (rotation * m_upDirection).Normalize();
	Dirty();
}

void FlybyCamera::Rotate(float x, float y, float z, float angleDegrees)
{
	Rotate(cd::Vec3f(x, y, z), angleDegrees);
}

void FlybyCamera::Yaw(float angleDegrees)
{
	Rotate(0.0f, 1.0f, 0.0f, angleDegrees);
}

void FlybyCamera::Pitch(float angleDegrees)
{
	Rotate(1.0f, 0.0f, 0.0f, angleDegrees);
}

void FlybyCamera::Roll(float angleDegrees)
{
	Rotate(0.0f, 0.0f, 1.0f, angleDegrees);
}

void FlybyCamera::YawLocal(float angleDegrees)
{
	Rotate(m_upDirection, angleDegrees);
}

void FlybyCamera::PitchLocal(float angleDegrees)
{
	Rotate(m_forwardDirection.Cross(m_upDirection).Normalize(), angleDegrees);
}

void FlybyCamera::RollLocal(float angleDegrees)
{
	Rotate(m_forwardDirection, angleDegrees);
}

}	// namespace engine