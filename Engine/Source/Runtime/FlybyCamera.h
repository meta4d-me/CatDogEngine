#pragma once

#include "Camera.h"

namespace engine
{

class FlybyCamera final : public Camera
{
public:
	using Camera::Camera;
	virtual ~FlybyCamera() = default;

	void MoveForward(const float amount);
	void MoveBackward(const float amount);
	void MoveLeft(const float amount);
	void MoveRight(const float amount);
	void MoveUp(const float amount);
	void MoveDown(const float amount);
	void Rotate(const bx::Vec3& axis, const float angleDegrees);
	void Rotate(const float x, const float y, const float z, const float angleDegrees);
	void Yaw(const float angleDegrees);
	void Pitch(const float angleDegrees);
	void Roll(const float angleDegrees);

	void YawLocal(const float angleDegrees);
	void PitchLocal(const float angleDegrees);
	void RollLocal(const float angleDegrees);
};

}	// namespace engine
