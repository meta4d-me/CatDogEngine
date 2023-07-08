#pragma once

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Math//Vector.hpp"
#include "Window/Input.h"

#include <cinttypes>

namespace engine
{

class IndustryCameraController
{
public:
	IndustryCameraController() = delete;
	~IndustryCameraController() = default;

	explicit IndustryCameraController(const SceneWorld* pSceneWorld, const float sensitivity, const float movement_speed);
	explicit IndustryCameraController(const SceneWorld* pSceneWorld, const float horizontal_sensitivity, const float vertical_sensitivity, const float movement_speed);

	IndustryCameraController(const IndustryCameraController&) = delete;
	IndustryCameraController(IndustryCameraController&&) = delete;
	IndustryCameraController& operator=(const IndustryCameraController&) = delete;
	IndustryCameraController& operator=(IndustryCameraController&&) = delete;

	void Update(float deltaTime);
	void RotateHorizon(float amount);
	void RotatedVertical(float amount);
	void ChangeDistance(float amount);

	void MoveForward(float amount);
	void MoveBackward(float amount);
	void MoveLeft(float amount);
	void MoveRight(float amount);
	void Pitch(float amount);
	void Yaw(float amount);

	void Rotate(const cd::Vec3f& axis, float angleDegrees);
	void Rotate(float x, float y, float z, float angleDegrees);

private:
	CameraComponent* GetMainCameraComponent() const;
	TransformComponent* GetTransformComponent() const;

private:
	const SceneWorld* m_pSceneWorld;

	cd::Vec3f m_cameraPosition;
	cd::Vec3f m_orbitCenter;
	float m_horizontalAngle = 0;
	float m_verticalAngle = 0;
	float m_distanceScale = 0;

	float m_horizontalSensitivity;
	float m_verticalSensitivity;
	float m_movementSpeed;

};
}