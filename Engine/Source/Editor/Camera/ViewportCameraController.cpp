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

ViewportCameraController::ViewportCameraController(const SceneWorld* pSceneWorld, float horizontal_sensitivity, float vertical_sensitivity, float movement_speed) :
	m_pSceneWorld(pSceneWorld),
	m_horizontalSensitivity(horizontal_sensitivity),
	m_verticalSensitivity(vertical_sensitivity),
	m_moveSpeed(movement_speed)
{
	assert(pSceneWorld);
}

bool ViewportCameraController::IsInAnimation() const
{
	return m_isFocusing;
}

bool ViewportCameraController::IsZooming() const
{
	return ImGui::GetIO().MouseWheel != 0.0f;
}

void ViewportCameraController::Zoom(float delta)
{
	float scaleDelta = delta * m_moveSpeed * 4.0f;
	m_distanceFromLookAt -= scaleDelta;
	m_eye = m_eye + m_lookAt * scaleDelta;
}

bool ViewportCameraController::IsPanning() const
{
	return ImGui::IsMouseDown(ImGuiMouseButton_Middle) &&
		!ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
		!ImGui::IsMouseDown(ImGuiMouseButton_Right);
}

void ViewportCameraController::Panning(float x, float y)
{
	MoveLeft(x);
	m_eye = m_eye + m_up * y;
}

bool ViewportCameraController::IsTurning() const
{
	if (ImGuizmo::IsUsing())
	{
		return false;
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		return ImGui::GetMouseDragDelta().x != 0.0f;
	}

	return false;
}

void ViewportCameraController::Turning(float x, float y)
{
	PitchLocal(m_verticalSensitivity * y * 0.01f);
	Yaw(m_horizontalSensitivity * x * 0.01f);
}

bool ViewportCameraController::IsTracking() const
{
	if (ImGuizmo::IsUsing())
	{
		return false;
	}

	return ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsKeyDown(ImGuiKey_Z);
}

void ViewportCameraController::Tracking(float x, float y)
{

}

bool ViewportCameraController::IsInWalkMode() const
{
	return ImGui::IsAnyMouseDown();
}

bool ViewportCameraController::Walking()
{
	bool dirty = false;
	if (ImGui::IsKeyPressed(ImGuiKey_W))
	{
		MoveForward(m_moveSpeed);
		dirty = true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_A))
	{
		MoveLeft(m_moveSpeed);
		dirty = true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_S))
	{
		MoveBackward(m_moveSpeed);
		dirty = true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_D))
	{
		MoveRight(m_moveSpeed);
		dirty = true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_E))
	{
		MoveDown(m_moveSpeed);
		dirty = true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Q))
	{
		MoveUp(m_moveSpeed);
		dirty = true;
	}

	return dirty;
}

bool ViewportCameraController::OnMouseMove(float x, float y)
{
	bool dirty = false;
	do
	{
		if (IsTracking())
		{
			Tracking(x, y);
			dirty = true;
			break;
		}

		if (IsPanning())
		{
			Panning(x, y);
			dirty = true;
			// While panning, should not do other opertions at the same time.
			break;
		}

		if (IsZooming())
		{
			Zoom(-y);
			dirty = true;
		}

		if (IsTurning())
		{
			Turning(x, y);
			dirty = true;
		}
	} while (false);

	return dirty;
}

bool ViewportCameraController::OnMouseWheel(float y)
{
	bool dirty = false;
	if (IsZooming())
	{
		Zoom(y);
		dirty = true;
	}

	if (ImGui::IsAnyMouseDown())
	{
		float speedRate = std::pow(2.0f, y / 10.0f);
		m_moveSpeed = speedRate * m_moveSpeed;
	}

	return dirty;
}

void ViewportCameraController::Update(float deltaTime)
{
	if (IsInAnimation())
	{
		// Camera is focusing an entity automatically.
		Focusing();
		return;
	}

	if (IsInControl())
	{
		CameraToController();
	}

	bool dirty = false;
	ImVec2 delta = ImGui::GetMouseDragDelta();
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		dirty |= OnMouseMove(delta.x, delta.y);
	}

	float scrollY = ImGui::GetIO().MouseWheel;
	if (scrollY != 0.0f)
	{
		dirty |= OnMouseWheel(scrollY);
	}

	if (IsInWalkMode())
	{
		dirty |= Walking();
	}

	if (dirty)
	{
		ControllerToCamera();
	}
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
	cd::Vec3f lookAt = m_lookAt.Normalize();
	cd::Vec3f up = m_up.Normalize();

	GetMainCameraComponent()->BuildViewMatrix(eye, lookAt, up);
	TransformComponent* pTransformComponent = GetMainCameraTransformComponent();
	pTransformComponent->GetTransform().SetTranslation(eye);
	
	cd::Vec3f rotationAxis = cd::Vec3f(0.0f, 0.0f, 1.0f).Cross(lookAt);
	float rotationAngle = std::acos(cd::Vec3f(0.0f, 0.0f, 1.0f).Dot(lookAt));
	pTransformComponent->GetTransform().SetRotation(cd::Quaternion::FromAxisAngle(rotationAxis, rotationAngle));
	pTransformComponent->Dirty();
	pTransformComponent->Build();
}

void ViewportCameraController::SetMovementSpeed(float speed)
{
	m_moveSpeed = speed;
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
}

void ViewportCameraController::MoveBackward(float amount)
{
	MoveForward(-amount);
}

void ViewportCameraController::MoveLeft(float amount)
{
	m_eye = m_eye + m_lookAt.Cross(m_up) * amount;
}

void ViewportCameraController::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void ViewportCameraController::MoveUp(float amount)
{
	m_eye = m_eye + cd::Vec3f(0.0f, 1.0f, 0.0f) * amount;
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
			m_moveSpeed = meshAABB.Size().Length() * 1.5f;
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

void ViewportCameraController::MoveToPosition(cd::Point position, cd::Vec3f rotation)
{
	m_isFocusing = true;
	m_eyeDestination = position;
	cd::Vec3f lookAt = cd::Quaternion::FromPitchYawRoll(rotation.x(), rotation.y(), rotation.z()) * cd::Vec3f(0.0f, 0.0f, 1.0f);
	m_lookAtDestination = lookAt.Normalize();
}

}