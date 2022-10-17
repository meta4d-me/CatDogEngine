#pragma once

#include "Camera.h"

namespace engine
{

class FlybyCamera : public Camera
{
public:
	FlybyCamera();
	FlybyCamera(const bx::Vec3& position);
	FlybyCamera(const bx::Vec3& position, const bx::Vec3& forward, const bx::Vec3& up);
	~FlybyCamera() = default;

	FlybyCamera(const FlybyCamera&) = delete;
	FlybyCamera(FlybyCamera&&) = delete;
	FlybyCamera& operator=(const FlybyCamera&) = delete;
	FlybyCamera& operator=(FlybyCamera&&) = delete;

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

	const bx::Vec3& GetPosition() const { return m_position; }
	const bx::Vec3& GetForwardDir() const { return m_forwardDirection; }
	const bx::Vec3& GetUpDir() const { return m_upDirection; }
	const float* GetViewMatrix() const;

	void Update() override;

private:
	bx::Vec3 m_position;
	bx::Vec3 m_forwardDirection;
	bx::Vec3 m_upDirection;
	float m_viewMatrix[16];
};

}	// namespace engine
