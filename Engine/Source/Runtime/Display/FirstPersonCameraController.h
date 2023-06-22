#pragma once

#include "Math/Vector.hpp"

namespace engine
{

class CameraComponent;
class SceneWorld;

class FirstPersonCameraController final
{
public:
	FirstPersonCameraController() = delete;
	explicit FirstPersonCameraController(const SceneWorld* pSceneWorld, const float sensitivity, const float movement_speed);
	explicit FirstPersonCameraController(const SceneWorld* pSceneWorld, const float horizontal_sensitivity, const float vertical_sensitivity, const float movement_speed);
	~FirstPersonCameraController() = default;

	FirstPersonCameraController(const FirstPersonCameraController&) = delete;
	FirstPersonCameraController(FirstPersonCameraController&&) = delete;
	FirstPersonCameraController& operator=(const FirstPersonCameraController&) = delete;
	FirstPersonCameraController& operator=(FirstPersonCameraController&&) = delete;

	void Update(float deltaTime);

	// Configs
	void SetMovementSpeed(const float speed);
	void SetSensitivity(const float horizontal, const float verticle);
	void SetHorizontalSensitivity(const float sensitivity);
	void SetVerticalSensitivity(const float sensitivity);

	// Operations
	void MoveForward(float amount);
	void MoveBackward(float amount);
	void MoveLeft(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);
	void MoveDown(float amount);
	void Rotate(const cd::Vec3f& axis, float angleDegrees);
	void Rotate(float x, float y, float z, float angleDegrees);
	void Yaw(float angleDegrees);
	void Pitch(float angleDegrees);
	void Roll(float angleDegrees);
	void YawLocal(float angleDegrees);
	void PitchLocal(float angleDegrees);
	void RollLocal(float angleDegrees);

private:
	engine::CameraComponent* GetMainCameraComponent() const;

private:
	const SceneWorld* m_pSceneWorld;

	float m_horizontalSensitivity;
	float m_verticalSensitivity;
	float m_movementSpeed;
};

}	// namespace engine