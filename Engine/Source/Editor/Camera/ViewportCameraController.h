#pragma once

#include "Camera/ICameraController.h"
#include "ECWorld/TransformComponent.h"
#include "Math/Box.hpp"
#include "Math/Transform.hpp"

namespace engine
{

class CameraComponent;
class SceneWorld;

class ViewportCameraController : public ICameraController
{
public:
	ViewportCameraController() = delete;
	explicit ViewportCameraController(const SceneWorld* pSceneWorld, float sensitivity, float movement_speed);
	explicit ViewportCameraController(const SceneWorld* pSceneWorld, float horizontal_sensitivity, float vertical_sensitivity, float movement_speed);
	~ViewportCameraController() = default;

	ViewportCameraController(const ViewportCameraController&) = delete;
	ViewportCameraController(ViewportCameraController&&) = delete;
	ViewportCameraController& operator=(const ViewportCameraController&) = delete;
	ViewportCameraController& operator=(ViewportCameraController&&) = delete;

	// Operations
	virtual bool IsInAnimation() const override;
	virtual bool IsZooming() const override;
	virtual bool IsPanning() const override;
	virtual bool IsTurning() const override;
	virtual bool IsTracking() const override;
	virtual bool IsInWalkMode() const override;

	// Event Handlers
	virtual void OnMouseDown() override;
	virtual void OnMouseUp() override;
	virtual void OnMouseMove(float x, float y) override;
	virtual void OnMouseWheel(float y) override;

	// TODO : EventDriven, not update.
	void Update(float deltaTime);

	// Synchronizes the controller to the transform of camera current state
	void CameraToController();

	// Synchronizes the transform of camera to the controller's current state
	void ControllerToCamera();

	// Implement the effect of a translation animation.
	void CameraFocus();
	void Focusing();

	//
	void Zoom(float delta);

	// Configs
	void SetMovementSpeed(float speed);
	void SetSensitivity(float horizontal, float verticle);
	void SetHorizontalSensitivity(float sensitivity);
	void SetVerticalSensitivity(float sensitivity);

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

	void Panning(float x, float y);

	// Circling Operations
	void AzimuthChanging(float amount);
	void ElevationChanging(float amount);



	// Synchronize view when begin using mayastyle camera or fpscamera
	void SynchronizeTrackingCamera();

	void MoveToPosition(cd::Point position, cd::Vec3f lookAt);

private:
	engine::CameraComponent* GetMainCameraComponent() const;
	engine::TransformComponent* GetMainCameraTransformComponent() const;
	const cd::Transform& GetMainCameraTransform();
	float CalculateZoomScale() { return std::max(std::abs(m_distanceFromLookAt), m_dollyThreshold); }

private:
	const SceneWorld* m_pSceneWorld;

	float m_dollyThreshold = 3.0f; // Maybe it should be depends on AABB
	float m_distanceFromLookAt = 100.0f;//distance from camera to LookAt point
	float m_azimuth = 0.0f;            //angle of rotation in XZ plane starting from +Z axis (radians)
	float m_elevation = 0.0f;           //angle of elevation from XZ plane (radians)

	float m_horizontalSensitivity = 0.0f;
	float m_verticalSensitivity = 0.0f;
	float m_movementSpeed = 0.0f;
	float m_initialMovemenSpeed = 0.0f;
	float m_mouseScroll = 0.0f;

	cd::Vec3f m_lookAtPoint = cd::Vec3f::Zero();
	cd::Vec3f m_lookAt = cd::Vec3f(0.0f, 1.0f, 0.0f);
	cd::Vec3f m_up = cd::Vec3f(0.0f, 0.0f, 1.0f);
	cd::Vec3f m_eye = cd::Vec3f::Zero();
	cd::Vec3f m_eyeDestination; // This is for focusing animation
	cd::Vec3f m_lookAtDestination;

	bool m_isTracking = false;
	bool m_isFocusing = false;
	bool m_isInWalkMode = false;
};

}