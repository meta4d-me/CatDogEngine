#pragma once

#include "ECWorld/TransformComponent.h"
#include "Math/Box.hpp"
#include "Math/Transform.hpp"
#include "Math/Vector.hpp"

namespace engine
{

class CameraComponent;
class SceneWorld;

class CameraController final
{
public:
	CameraController() = delete;
	explicit CameraController(const SceneWorld* pSceneWorld, const float sensitivity, const float movement_speed);
	explicit CameraController(const SceneWorld* pSceneWorld, const float horizontal_sensitivity, const float vertical_sensitivity, const float movement_speed);
	~CameraController() = default;

	CameraController(const CameraController&) = delete;
	CameraController(CameraController&&) = delete;
	CameraController& operator=(const CameraController&) = delete;
	CameraController& operator=(CameraController&&) = delete;

	void Update(float deltaTime);

	// Configs
	void SetMovementSpeed(const float speed);
	void SetSensitivity(const float horizontal, const float verticle);
	void SetHorizontalSensitivity(const float sensitivity);
	void SetVerticalSensitivity(const float sensitivity);

	// Fps Operations
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

	//Circling Operations
	void AzimuthChanging(float amount);
	void ElevationChanging(float amount);
	//Double Click entity,camera will focus
	void  CameraFocus(const cd::AABB& aabb);
	//Implement the effect of a translation animation.
	void Focusing();
	//Synchronizes the controller to the transform of camera current state
	void CameraToController();
	//Synchronizes the transform of camera to the controller's current state
	void ControllerToCamera();
	//Synchronize view when begin using mayastyle camera or fpscamera
	void SynchronizeMayaCamera();
	//When using camerafocus() mayastyle should be true
	void SetMayaStyle() { m_isMayaStyle = true; }
private:
	engine::CameraComponent* GetMainCameraComponent() const;
	engine::TransformComponent* GetMainCameraTransformComponent() const;
	cd::Transform GetMainCameraTransform();
	float CalculateZoomScale() { return std::max(std::abs(m_distanceFromLookAt), m_dollyThreshold); }

private:
	const SceneWorld* m_pSceneWorld;

	float m_dollyThreshold = 3.0f; // Maybe it should be depends on AABB
	float m_distanceFromLookAt = 100.0f;//distance from camera to LookAt point
	float m_azimuth;            //angle of rotation in XZ plane starting from +Z axis (radians)
	float m_elevation;           //angle of elevation from XZ plane (radians)

	float m_horizontalSensitivity;
	float m_verticalSensitivity;
	float m_movementSpeed;
	float m_initialMovemenSpeed;
	float m_mouseScroll = 0;

	cd::Vec3f m_lookAtPoint = cd::Vec3f::Zero();
	cd::Vec3f m_lookAt;
	cd::Vec3f m_up;
	cd::Vec3f m_eye;
	cd::Vec3f m_eyeDestination; // This is for focusing animation

	bool m_isMayaStyle;
	bool m_isFocusing = false;
};

}	// namespace engine