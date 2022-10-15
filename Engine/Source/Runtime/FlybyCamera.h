#pragma once

#include "Camera.h"

namespace engine
{

class FlybyCamera : public Camera
{
public:
	FlybyCamera();
	FlybyCamera(const bx::Vec3& position);
	FlybyCamera(const bx::Vec3& position, const bx::Quaternion& orientation);
	~FlybyCamera() = default;

	FlybyCamera(const FlybyCamera&) = delete;
	FlybyCamera(FlybyCamera&&) = delete;
	FlybyCamera& operator=(const FlybyCamera&) = delete;
	FlybyCamera& operator=(FlybyCamera&&) = delete;

	void Translate(const bx::Vec3& v);
	void Translate(const float x, const float y, const float z);
	void Rotate(const bx::Vec3& axis, const float angleDegrees);
	void Rotate(const float x, const float y, const float z, const float angleDegrees);
	void Yaw(const float angleDegrees);
	void Pitch(const float angleDegrees);
	void Roll(const float angleDegrees);

	void RotateLocal(const bx::Vec3& axis, const float angleDegrees);
	void RotateLocal(const float x, const float y, const float z, const float angleDegrees);
	void YawLocal(const float angleDegrees);
	void PitchLocal(const float angleDegrees);
	void RollLocal(const float angleDegrees);

	const bx::Vec3& GetPosition() const { return m_position; }
	const bx::Quaternion& GetOrientation() const { return m_orientation; }
	const float* GetViewMatrix() const;

	void Update() override;

private:
	bx::Vec3		m_position;
	bx::Quaternion	m_orientation;
	float			m_viewMatrix[16];
};

}	// namespace engine
