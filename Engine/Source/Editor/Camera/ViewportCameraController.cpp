#include "ViewportCameraController.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "Math/Quaternion.hpp"
#include "Window/Input.h"

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>

#include <cassert>
#include <cmath>

namespace engine
{

ViewportCameraController::ViewportCameraController(
	const SceneWorld* pSceneWorld,
	float sensitivity,
	float movement_speed)
	: ViewportCameraController(pSceneWorld, sensitivity, sensitivity, movement_speed)
{
}

ViewportCameraController::ViewportCameraController(
	const SceneWorld* pSceneWorld,
	float horizontal_sensitivity,
	float vertical_sensitivity,
	float movement_speed)
	: m_pSceneWorld(pSceneWorld)
	, m_horizontalSensitivity(horizontal_sensitivity)
	, m_verticalSensitivity(vertical_sensitivity)
	, m_movementSpeed(movement_speed)
	, m_initialMovemenSpeed(movement_speed)
{
	assert(pSceneWorld);
}

bool ViewportCameraController::IsInAnimation() const
{
	return m_isFocusing;
}

bool ViewportCameraController::IsZooming() const
{
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		return Input::Get().GetMousePositionOffsetY() != 0.0f;
	}

	return Input::Get().GetMouseScrollOffsetY() != 0.0f;
}

bool ViewportCameraController::IsPanning() const
{
	return ImGui::IsMouseDown(ImGuiMouseButton_Middle);
}

bool ViewportCameraController::IsTurning() const
{
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		// Offset Y is used to zoom in/out when left mouse button down.
		return Input::Get().GetMousePositionOffsetX() != 0.0f;
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		return Input::Get().GetMousePositionOffsetX() != 0.0f || Input::Get().GetMousePositionOffsetY() != 0.0f;
	}

	return false;
}

bool ViewportCameraController::IsTracking() const
{
	return ImGui::IsMouseDown(ImGuiMouseButton_Right) && ImGui::IsKeyDown(ImGuiKey_Z);
}

bool ViewportCameraController::IsInWalkMode() const
{
	return m_isInWalkMode;
}

void ViewportCameraController::OnMouseDown()
{
	if (ImGui::IsAnyMouseDown())
	{
		m_isInWalkMode = true;
	}

	CameraToController();
}

void ViewportCameraController::OnMouseUp()
{
	m_isInWalkMode = false;
}

void ViewportCameraController::OnMouseMove(float x, float y)
{
	bool dirty = false;
	if (IsZooming())
	{
		Zoom(y);
		dirty = true;
	}

	if (dirty)
	{
		ControllerToCamera();
	}
}

void ViewportCameraController::OnMouseWheel(float y)
{
	bool dirty = false;
	if (IsZooming())
	{
		Zoom(y);
		dirty = true;
	}

	if (dirty)
	{
		ControllerToCamera();
	}
}

void ViewportCameraController::Update(float deltaTime)
{
	if (IsInAnimation())
	{
		// Camera is focusing an entity automatically.
		return;
	}

	float offsetX = static_cast<float>(Input::Get().GetMousePositionOffsetX());
	float offsetY = static_cast<float>(Input::Get().GetMousePositionOffsetY());
	if (offsetX != 0.0f || offsetY != 0.0f)
	{
		OnMouseMove(offsetX, offsetY);
	}

	float scrollY = Input::Get().GetMouseScrollOffsetY();
	if (scrollY != 0.0f)
	{
		OnMouseWheel(scrollY);
	}

	//if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
	//{
	//	m_isTracking = false;
	//	float dx = Input::Get().GetMousePositionOffsetX() * deltaTime * m_movementSpeed / 4.0f;
	//	float dy = Input::Get().GetMousePositionOffsetY() * deltaTime * m_movementSpeed / 4.0f;
	//	Panning(dx, dy);
	//}

	//if (ImGui::IsKeyPressed(ImGuiKey_Z, false))
	//{
	//	// TODO : Only need to happen once in the first time press z.
	//	SynchronizeTrackingCamera();
	//
	//	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !m_isMoving)
	//	{
	//		m_isTracking = true;
	//		ElevationChanging(m_verticalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
	//		AzimuthChanging(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	//	}
	//
	//	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	//	{
	//		float scaleDelta = (Input::Get().GetMousePositionOffsetX() - Input::Get().GetMousePositionOffsetY()) * deltaTime * m_movementSpeed;
	//		m_distanceFromLookAt -= scaleDelta;
	//		m_eye = m_eye + m_lookAt * scaleDelta;
	//		ControllerToCamera();
	//	}
	//
	//	return;
	//}
	//if (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Middle))
	//{
	//	if (Input::Get().GetMouseScrollOffsetY())
	//	{
	//		float speedRate = std::pow(2.0f, Input::Get().GetMouseScrollOffsetY() / 10.0f);
	//		m_movementSpeed = speedRate * m_movementSpeed;
	//	}
	//
	//	if (ImGui::IsKeyPressed(ImGuiKey_W) && !m_isMoving)
	//	{
	//		m_isTracking = false;
	//		MoveForward(m_movementSpeed * deltaTime);
	//	}
	//
	//	if (ImGui::IsKeyPressed(ImGuiKey_A) && !m_isMoving)
	//	{
	//		m_isTracking = false;
	//		MoveLeft(m_movementSpeed * deltaTime);
	//	}
	//
	//	if (ImGui::IsKeyPressed(ImGuiKey_S) && !m_isMoving)
	//	{
	//		m_isTracking = false;
	//		MoveBackward(m_movementSpeed * deltaTime);
	//	}
	//
	//	if (ImGui::IsKeyPressed(ImGuiKey_D) && !m_isMoving)
	//	{
	//		m_isTracking = false;
	//		MoveRight(m_movementSpeed * deltaTime);
	//	}
	//
	//	if (ImGui::IsKeyPressed(ImGuiKey_E))
	//	{
	//		m_isTracking = false;
	//		MoveDown(m_movementSpeed * deltaTime);
	//	}
	//
	//	if (ImGui::IsKeyPressed(ImGuiKey_Q) && !m_isMoving)
	//	{
	//		m_isTracking = false;
	//		MoveUp(m_movementSpeed * deltaTime);
	//	}
	//}
	//
	//	
	//if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && !m_isMoving)
	//{
	//	m_isTracking = false;
	//	Yaw(m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	//}
	//
	//if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGuizmo::IsUsing())
	//{
	//	m_isTracking = false;
	//	PitchLocal(m_verticalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
	//	Yaw(m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	//}
}

void ViewportCameraController::CameraToController()
{
	m_eye = GetMainCameraTransform().GetTranslation();
	m_lookAt = CameraComponent::GetLookAt(GetMainCameraTransform());
	m_up = CameraComponent::GetUp(GetMainCameraTransform());
}

void ViewportCameraController::ControllerToCamera()
{
	cd::Vec3f eye = m_eye;
	cd::Vec3f lookAt = m_lookAt;
	cd::Vec3f up = m_up;

	if (m_isTracking)
	{
		float sinPhi = std::sin(m_elevation);
		float cosPhi = std::cos(m_elevation);
		float sinTheta = std::sin(m_azimuth);
		float cosTheta = std::cos(m_azimuth);

		lookAt = cd::Vec3f(-cosPhi * sinTheta, -sinPhi, -cosPhi * cosTheta);
		lookAt.Normalize();
		cd::Vec3f cross = cd::Vec3f(cosTheta, 0.0f, -sinTheta);
		up = cross.Cross(lookAt);

		float lookAtOffset = 0.0f;
		if (m_distanceFromLookAt < m_dollyThreshold)
		{
			lookAtOffset = m_distanceFromLookAt - m_dollyThreshold;
		}

		float eyeOffset = m_distanceFromLookAt;
		eye = m_lookAtPoint - (lookAt * eyeOffset);

		// Synchronize view to fps camera
		m_eye = eye;
		m_lookAt = lookAt;
		m_up = up;
	}

	GetMainCameraComponent()->BuildViewMatrix(eye, lookAt, up);
	TransformComponent* pTransformComponent = GetMainCameraTransformComponent();
	pTransformComponent->GetTransform().SetTranslation(eye);
	
	cd::Vec3f rotationAxis = cd::Vec3f(0.0f, 0.0f,1.0f).Cross(lookAt).Normalize();
	float rotationAngle = std::acos(cd::Vec3f(0.0f, 0.0f, 1.0f).Dot(lookAt));
	pTransformComponent->GetTransform().SetRotation(cd::Quaternion::FromAxisAngle(rotationAxis, rotationAngle));
	pTransformComponent->Build();
}

void ViewportCameraController::SetMovementSpeed(float speed)
{
	m_movementSpeed = speed;
}

void ViewportCameraController::SetSensitivity(float horizontal, float verticle)
{
	m_horizontalSensitivity = horizontal;
	m_verticalSensitivity = verticle;
}

void ViewportCameraController::SetHorizontalSensitivity(float sensitivity)
{
	m_horizontalSensitivity = sensitivity;
}

void ViewportCameraController::SetVerticalSensitivity(float sensitivity)
{
	m_verticalSensitivity = sensitivity;
}

engine::CameraComponent* ViewportCameraController::GetMainCameraComponent() const
{
	return m_pSceneWorld->GetCameraComponent(m_pSceneWorld->GetMainCameraEntity());
}

engine::TransformComponent* ViewportCameraController::GetMainCameraTransformComponent() const
{
	return m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity());
}

const cd::Transform& ViewportCameraController::GetMainCameraTransform() 
{
	return m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity())->GetTransform();
}

void ViewportCameraController::MoveForward(float amount)
{
	m_eye = m_eye + m_lookAt * amount;
	ControllerToCamera();
}

void ViewportCameraController::MoveBackward(float amount)
{
	MoveForward(-amount);
}

void ViewportCameraController::MoveLeft(float amount)
{
	m_eye = m_eye + m_lookAt.Cross(m_up) * amount;
	ControllerToCamera();
}

void ViewportCameraController::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void ViewportCameraController::MoveUp(float amount)
{
	m_eye = m_eye + cd::Vec3f(0.0f, 1.0f, 0.0f) * amount;
	ControllerToCamera();
}

void ViewportCameraController::MoveDown(float amount)
{
	MoveUp(-amount);
}

void ViewportCameraController::Rotate(const cd::Vec3f& axis, float angleDegrees)
{
	cd::Quaternion rotation = cd::Quaternion::FromAxisAngle(axis, cd::Math::DegreeToRadian<float>(angleDegrees));
	m_lookAt = rotation * m_lookAt;
	m_up = rotation * m_up;
	ControllerToCamera();
}

void ViewportCameraController::Rotate(float x, float y, float z, float angleDegrees)
{
	Rotate(cd::Vec3f(x, y, z), angleDegrees);
}

void ViewportCameraController::Yaw(float angleDegrees)
{
	Rotate(0.0f, 1.0f, 0.0f, angleDegrees);
}

void ViewportCameraController::Pitch(float angleDegrees)
{
	Rotate(1.0f, 0.0f, 0.0f, angleDegrees);
}

void ViewportCameraController::Roll(float angleDegrees)
{
	Rotate(0.0f, 0.0f, 1.0f, angleDegrees);
}

void ViewportCameraController::YawLocal(float angleDegrees)
{
	Rotate(m_up, angleDegrees);
}

void ViewportCameraController::PitchLocal(float angleDegrees)
{
	Rotate(m_up.Cross(m_lookAt), angleDegrees);
}

void ViewportCameraController::RollLocal(float angleDegrees)
{
	Rotate(m_lookAt, angleDegrees);
}

void ViewportCameraController::ElevationChanging(float angleDegrees)
{
	m_elevation += angleDegrees / 360.0f * cd::Math::PI;
	if (m_elevation > cd::Math::PI)
	{
		m_elevation -= cd::Math::TWO_PI;
	}
	else if (m_elevation < -cd::Math::PI)
	{
		m_elevation += cd::Math::TWO_PI;
	}
	ControllerToCamera();
}

void ViewportCameraController::AzimuthChanging(float angleDegrees)
{
	m_azimuth -= angleDegrees / 360.0f * cd::Math::PI;
	if (m_azimuth > cd::Math::PI)
	{
		m_azimuth -= cd::Math::TWO_PI;
	}
	else if (m_azimuth < -cd::Math::PI)
	{
		m_azimuth += cd::Math::TWO_PI;
	}
	ControllerToCamera();
}

void ViewportCameraController::SynchronizeTrackingCamera()
{
	m_lookAtPoint = m_lookAt * m_distanceFromLookAt + m_eye;
	m_elevation = std::asin(-m_lookAt.y());

	if (cd::Math::IsSmallThanZero(m_up.y()))
	{
		if (cd::Math::IsLargeThanZero(m_lookAt.y()))
		{
			m_elevation = -cd::Math::PI - m_elevation;
		}
		else
		{
			m_elevation = cd::Math::PI - m_elevation;
			m_azimuth = std::atan2(m_lookAt.x(), m_lookAt.z());
		}
	}
	else
	{
		m_azimuth = std::atan2(-m_lookAt.x(), -m_lookAt.z());
	}
}

void ViewportCameraController::CameraFocus()
{
	Entity selectedEntity = m_pSceneWorld->GetSelectedEntity();
	if (selectedEntity == INVALID_ENTITY)
	{
		return;
	}

	if (TransformComponent* pTransform = m_pSceneWorld->GetTransformComponent(selectedEntity))
	{
		m_isFocusing = true;
		if (CollisionMeshComponent* pCollisionMesh = m_pSceneWorld->GetCollisionMeshComponent(selectedEntity))
		{
			cd::AABB meshAABB = pCollisionMesh->GetAABB();
			meshAABB = meshAABB.Transform(pTransform->GetWorldMatrix());
			m_distanceFromLookAt = (meshAABB.Max() - meshAABB.Center()).Length() * 3.0f;
			m_eyeDestination = meshAABB.Center() - m_lookAt * m_distanceFromLookAt;
			m_movementSpeed = meshAABB.Size().Length() * 1.5f;
		}
		else
		{
			m_eyeDestination = pTransform->GetTransform().GetTranslation() - m_lookAt * m_distanceFromLookAt;
		}
	}
}

void ViewportCameraController::Focusing()
{
	if (m_isFocusing)
	{
		if (cd::Math::IsSmallThan((m_eye - m_eyeDestination).Length(), 0.01f))
		{
			m_isFocusing = false;
		}
		else
		{
			cd::Direction eyeMove = (m_eye - m_eyeDestination).Normalize();
			float stepDistance = (m_eye - m_eyeDestination).Length() / 5.0f;
			m_eye = m_eye - eyeMove * stepDistance;

			SynchronizeTrackingCamera();
			ControllerToCamera();
		}
	}
}

void ViewportCameraController::Zoom(float delta)
{
	float scaleDelta = delta * m_movementSpeed;
	m_distanceFromLookAt -= scaleDelta;
	m_eye = m_eye + m_lookAt * scaleDelta;
}

void ViewportCameraController::Panning(float x, float y)
{
	MoveLeft(x);
	m_eye = m_eye + m_up * y;
}

void ViewportCameraController::MoveToPosition(cd::Point position, cd::Vec3f rotation)
{
	m_isFocusing = true;
	m_eyeDestination = position;
	cd::Vec3f lookAt = cd::Quaternion::FromPitchYawRoll(rotation.x(), rotation.y(), rotation.z()) * cd::Vec3f(0.0f, 0.0f, 1.0f);
	m_lookAtDestination = lookAt.Normalize();
}

}