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
	explicit ViewportCameraController(const SceneWorld* pSceneWorld, float speed);
	ViewportCameraController(const ViewportCameraController&) = delete;
	ViewportCameraController(ViewportCameraController&&) = delete;
	ViewportCameraController& operator=(const ViewportCameraController&) = delete;
	ViewportCameraController& operator=(ViewportCameraController&&) = delete;
	virtual ~ViewportCameraController() = default;

	// Synchronizes the controller to the transform of camera current state
	void CameraToController();

	// Synchronizes the transform of camera to the controller's current state
	void ControllerToCamera();

	// Operations
	virtual bool IsInAnimation() const override;
	void CameraFocus();
	void Focusing();

	virtual bool IsZooming() const override;
	virtual bool IsPanning() const override;
	virtual bool IsTurning() const override;
	virtual bool IsTracking() const override;
	virtual bool IsInWalkMode() const override;

	// Event Handlers
	bool OnMouseDown(float x, float y);
	bool OnMouseUp(float x, float y);
	bool OnMouseMove(float x, float y);
	bool OnMouseWheel(float delta);
	bool OnKeyDown();

	void MoveToPosition(cd::Point position, cd::Vec3f lookAt);

private:
	engine::CameraComponent* GetMainCameraComponent() const;
	engine::TransformComponent* GetMainCameraTransformComponent() const;
	const cd::Transform& GetMainCameraTransform();
	float CalculateZoomScale() const { return std::max(std::abs(m_distanceFromLookAt), m_dollyThreshold); }
	float GetWalkSpeed() const { return m_walkSpeed * m_walkSpeedScale; }

private:
	const SceneWorld* m_pSceneWorld;

	cd::Vec2f m_lastMousePoint = cd::Vec2f::Zero();
	bool m_dragging = false;

	float m_dollyThreshold = 3.0f; // Maybe it should be depends on AABB
	float m_distanceFromLookAt = 100.0f;//distance from camera to LookAt point
	float m_azimuth = 0.0f;            //angle of rotation in XZ plane starting from +Z axis (radians)
	float m_elevation = 0.0f;           //angle of elevation from XZ plane (radians)

	float m_walkSpeed = 0.0f;
	float m_walkSpeedScale = 1.0f;

	cd::Vec3f m_lookAtPoint = cd::Vec3f::Zero();
	cd::Vec3f m_lookAt = cd::Vec3f(0.0f, 1.0f, 0.0f);
	cd::Vec3f m_up = cd::Vec3f(0.0f, 0.0f, 1.0f);
	cd::Vec3f m_eye = cd::Vec3f::Zero();
	
	cd::Vec3f m_eyeDestination; // This is for focusing animation
	cd::Vec3f m_lookAtDestination;

	bool m_isFocusing = false;
	bool m_isTracking = false;
};

}