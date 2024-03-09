#include "CameraController.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "Math/Quaternion.hpp"
#include "Window/Input.h"

#include <ImGui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>

#include <cassert>
#include <cmath>

namespace engine
{

CameraController::CameraController(
	const SceneWorld* pSceneWorld,
	float sensitivity,
	float movement_speed)
	: CameraController(pSceneWorld, sensitivity, sensitivity, movement_speed)
{
}

CameraController::CameraController(
	const SceneWorld* pSceneWorld,
	float horizontal_sensitivity,
	float vertical_sensitivity,
	float movement_speed)
	: m_pSceneWorld(pSceneWorld)
	, m_horizontalSensitivity(horizontal_sensitivity)
	, m_verticalSensitivity(vertical_sensitivity)
	, m_movementSpeed(movement_speed)
{
	assert(pSceneWorld);
}

void CameraController::CameraToController()
{
	m_eye = GetMainCameraTransform().GetTranslation();
	m_lookAt = CameraComponent::GetLookAt(GetMainCameraTransform());
	m_up = CameraComponent::GetUp(GetMainCameraTransform());
}

void CameraController::ControllerToCamera()
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
		cd::Vec3f cross = cd::Vec3f(cosTheta, 0.0f, -sinTheta);
		up = cross.Cross(lookAt);

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
	
	// TODO : Add interface to math lib.
	lookAt.Normalize();
	cd::Vec3f rotationAxis = cd::Vec3f(0.0f, 0.0f,1.0f).Cross(lookAt).Normalize();
	float rotationAngle = std::acos(cd::Vec3f(0.0f, 0.0f, 1.0f).Dot(lookAt));
	pTransformComponent->GetTransform().SetRotation(cd::Quaternion::FromAxisAngle(rotationAxis, rotationAngle));
	pTransformComponent->Build();
}

void CameraController::Update(float deltaTime)
{
	Moving();
	bool isAnyMouseButtonPressed = engine::Input::Get().IsMouseLBPressed() || engine::Input::Get().IsMouseMBPressed() || engine::Input::Get().IsMouseRBPressed();
	bool isAnyDirectionMouseMoved = 0 != engine::Input::Get().GetMousePositionOffsetX() || 0 != engine::Input::Get().GetMousePositionOffsetY();
	m_isMouseMovedInView = isAnyMouseButtonPressed && isAnyDirectionMouseMoved;

	if (Input::Get().IsKeyPressed(KeyCode::z))
	{
		// TODO : Only need to happen once in the first time press z.
		SynchronizeTrackingCamera();

		if (Input::Get().IsMouseLBPressed() && !m_isMoving)
		{
			m_isTracking = true;
			ElevationChanging(m_verticalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
			AzimuthChanging(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
		}

		if (Input::Get().IsMouseRBPressed())
		{
			float scaleDelta = (Input::Get().GetMousePositionOffsetX() - Input::Get().GetMousePositionOffsetY()) * deltaTime * m_movementSpeed;
			m_distanceFromLookAt -= scaleDelta;
			m_eye = m_eye + m_lookAt * scaleDelta;
			ControllerToCamera();
		}

		if (Input::Get().GetMouseScrollOffsetY())
		{
			float scaleDelta = Input::Get().GetMouseScrollOffsetY() * deltaTime * m_movementSpeed * 5.0f;

			m_distanceFromLookAt -= scaleDelta;
			m_eye = m_eye + m_lookAt * scaleDelta;
			ControllerToCamera();
		}
	}
	else
	{
		if (Input::Get().IsMouseMBPressed())
		{
			m_isTracking = false;
			float dx = Input::Get().GetMousePositionOffsetX() * deltaTime * m_movementSpeed / 4.0f;
			float dy = Input::Get().GetMousePositionOffsetY() * deltaTime * m_movementSpeed / 4.0f;
			Panning(dx, dy);
		}
		if (Input::Get().IsMouseLBPressed() || Input::Get().IsMouseRBPressed() || Input::Get().IsMouseMBPressed())
		{
			if (Input::Get().GetMouseScrollOffsetY())
			{
				float speedRate = std::pow(2.0f, Input::Get().GetMouseScrollOffsetY() / 10.0f);
				m_movementSpeed = speedRate * m_movementSpeed;
			}

			if (Input::Get().IsKeyPressed(KeyCode::w) && !m_isMoving)
			{
				m_isTracking = false;
				MoveForward(m_movementSpeed * deltaTime);
			}

			if (Input::Get().IsKeyPressed(KeyCode::a) && !m_isMoving)
			{
				m_isTracking = false;
				MoveLeft(m_movementSpeed * deltaTime);
			}

			if (Input::Get().IsKeyPressed(KeyCode::s) && !m_isMoving)
			{
				m_isTracking = false;
				MoveBackward(m_movementSpeed * deltaTime);
			}

			if (Input::Get().IsKeyPressed(KeyCode::d) && !m_isMoving)
			{
				m_isTracking = false;
				MoveRight(m_movementSpeed * deltaTime);
			}

			if (Input::Get().IsKeyPressed(KeyCode::q))
			{
				m_isTracking = false;
				MoveDown(m_movementSpeed * deltaTime);
			}

			if (Input::Get().IsKeyPressed(KeyCode::e) && !m_isMoving)
			{
				m_isTracking = false;
				MoveUp(m_movementSpeed * deltaTime);
			}
		}

		
		if (Input::Get().IsMouseLBPressed() && !m_isMoving && m_isFirstClickInViewScene && !ImGuizmo::IsUsing())
		{
			m_isTracking = false;
			//MoveFront(m_movementSpeed * Input::Get().GetMousePositionOffsetY() * deltaTime);
			Yaw(m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
		}

		if (Input::Get().IsMouseRBPressed() && !m_isMoving && m_isFirstClickInViewScene && !ImGuizmo::IsUsing())
		{
			m_isTracking = false;
			PitchLocal(m_verticalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
			Yaw(m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
		}
		if (Input::Get().GetMouseScrollOffsetY() && !m_isMoving && m_isMouseInViewScene && !ImGuizmo::IsUsing() && !Input::Get().IsMouseLBPressed() && !Input::Get().IsMouseRBPressed())
		{
			m_isTracking = false;
			MoveForward(m_movementSpeed * Input::Get().GetMouseScrollOffsetY() * deltaTime * 10.0f);
		}
	}
}

void CameraController::SetMovementSpeed(float speed)
{
	m_movementSpeed = speed;
}

void CameraController::SetSensitivity(float horizontal, float verticle)
{
	m_horizontalSensitivity = horizontal;
	m_verticalSensitivity = verticle;
}

void CameraController::SetHorizontalSensitivity(float sensitivity)
{
	m_horizontalSensitivity = sensitivity;
}

void CameraController::SetVerticalSensitivity(float sensitivity)
{
	m_verticalSensitivity = sensitivity;
}

engine::CameraComponent* CameraController::GetMainCameraComponent() const
{
	return m_pSceneWorld->GetCameraComponent(m_pSceneWorld->GetMainCameraEntity());
}

engine::TransformComponent* CameraController::GetMainCameraTransformComponent() const
{
	return m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity());
}

const cd::Transform& CameraController::GetMainCameraTransform() 
{
	return m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity())->GetTransform();
}

void CameraController::MoveForward(float amount)
{
	m_eye = m_eye + m_lookAt * amount;
	ControllerToCamera();
}

void CameraController::MoveBackward(float amount)
{
	MoveForward(-amount);
}

void CameraController::MoveLeft(float amount)
{
	m_eye = m_eye + m_lookAt.Cross(m_up) * amount;
	ControllerToCamera();
}

void CameraController::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void CameraController::MoveUp(float amount)
{
	m_eye = m_eye + cd::Vec3f(0.0f, 1.0f, 0.0f) * amount;
	ControllerToCamera();
}

void CameraController::MoveDown(float amount)
{
	MoveUp(-amount);
}

void CameraController::MoveFront(float amount)
{
	cd::Vec3f direction = cd::Vec3f(m_lookAt.x(), 0.0f, m_lookAt.z()).Normalize();
	m_eye = m_eye - direction * amount;
	ControllerToCamera();
}

void CameraController::MoveBack(float amount)
{
	MoveFront(-amount);
}

void CameraController::Rotate(const cd::Vec3f& axis, float angleDegrees)
{
	cd::Quaternion rotation = cd::Quaternion::FromAxisAngle(axis, cd::Math::DegreeToRadian<float>(angleDegrees));
	m_lookAt = rotation * m_lookAt;
	m_up = rotation * m_up;
	ControllerToCamera();
}

void CameraController::Rotate(float x, float y, float z, float angleDegrees)
{
	Rotate(cd::Vec3f(x, y, z), angleDegrees);
}


void CameraController::Yaw(float angleDegrees)
{
	Rotate(0.0f, 1.0f, 0.0f, angleDegrees);
}

void CameraController::Pitch(float angleDegrees)
{
	Rotate(1.0f, 0.0f, 0.0f, angleDegrees);
}

void CameraController::Roll(float angleDegrees)
{
	Rotate(0.0f, 0.0f, 1.0f, angleDegrees);
}

void CameraController::YawLocal(float angleDegrees)
{
	Rotate(m_up, angleDegrees);
}

void CameraController::PitchLocal(float angleDegrees)
{
	Rotate(m_up.Cross(m_lookAt), angleDegrees);
}

void CameraController::RollLocal(float angleDegrees)
{
	Rotate(m_lookAt, angleDegrees);
}

void CameraController::ElevationChanging(float angleDegrees)
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

void CameraController::AzimuthChanging(float angleDegrees)
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

void CameraController::SynchronizeTrackingCamera()
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

void CameraController::CameraFocus()
{
	Entity selectedEntity = m_pSceneWorld->GetSelectedEntity();
	if (selectedEntity == INVALID_ENTITY)
	{
		return;
	}
	if (TransformComponent* pTransform = m_pSceneWorld->GetTransformComponent(selectedEntity))
	{
		m_isMoving = true;
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

void CameraController::Moving()
{
	if (m_isMoving)
	{
		if (cd::Math::IsSmallThan((m_eye - m_eyeDestination).Length(), 0.01f))
		{
			m_isMoving = false;
			return;
		}
		cd::Direction eyeMove = (m_eye - m_eyeDestination).Normalize();
		float stepDistance = (m_eye - m_eyeDestination).Length() / 5.0f;
		m_eye = m_eye - eyeMove * stepDistance;

		SynchronizeTrackingCamera();
		ControllerToCamera();
	}
}

void CameraController::Panning(float x, float y)
{
	MoveLeft(x);
	m_eye = m_eye + m_up * y;
}

void CameraController::MoveToPosition(cd::Point position, cd::Vec3f rotation)
{
	m_isMoving = true;
	m_eyeDestination = position;
	cd::Vec3f lookAt = cd::Quaternion::FromPitchYawRoll(rotation.x(), rotation.y(), rotation.z()) * cd::Vec3f(0.0f, 0.0f, 1.0f);
	m_lookAtDestination = lookAt.Normalize();
}

}