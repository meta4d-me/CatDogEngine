#pragma once

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Math//Vector.hpp"
#include "Window/Input.h"

namespace engine
{

class CameraController
{
public:
	CameraController() = delete;
	~CameraController() = default;

	explicit CameraController(const SceneWorld* pSceneWorld, const float sensitivity, const float movement_speed);
	explicit CameraController(const SceneWorld* pSceneWorld, const float horizontal_sensitivity, const float vertical_sensitivity, const float movement_speed);

	CameraController(const CameraController&) = delete;
	CameraController(CameraController&&) = delete;
	CameraController& operator=(const CameraController&) = delete;
	CameraController& operator=(CameraController&&) = delete;

	void Update(float deltaTime);
	void Rotate(float amount);

private:
	CameraComponent* GetMainCameraComponent() const;
	TransformComponent* GetTransformComponent() const;

private:
	const SceneWorld* m_pSceneWorld;

	cd::Vec3f m_cameraPosition;
	cd::Vec3f m_orbitCenter;
	float m_horizontalAngle = 0;
	float m_verticalAngle = 0;
	float m_movementSpeed;

};
}