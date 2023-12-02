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

ViewportCameraController::ViewportCameraController(const SceneWorld* pSceneWorld, float speed) :
	m_pSceneWorld(pSceneWorld),
	m_walkSpeed(speed)
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

bool ViewportCameraController::IsPanning() const
{
	return ImGui::IsMouseDown(ImGuiMouseButton_Middle) &&
		!ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
		!ImGui::IsMouseDown(ImGuiMouseButton_Right);
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

bool ViewportCameraController::IsTracking() const
{
	if (ImGuizmo::IsUsing())
	{
		return false;
	}

	return ImGui::IsMouseDown(ImGuiMouseButton_Left) && engine::Input::Get().ContainsModifier(engine::KeyMod::KMOD_ALT);
}

bool ViewportCameraController::IsInWalkMode() const
{
	bool isAnyKeyDown = ImGui::IsKeyPressed(ImGuiKey_W) || ImGui::IsKeyPressed(ImGuiKey_UpArrow) ||
		ImGui::IsKeyPressed(ImGuiKey_A) || ImGui::IsKeyPressed(ImGuiKey_LeftArrow) ||
		ImGui::IsKeyPressed(ImGuiKey_S) || ImGui::IsKeyPressed(ImGuiKey_DownArrow) ||
		ImGui::IsKeyPressed(ImGuiKey_D) || ImGui::IsKeyPressed(ImGuiKey_RightArrow) ||
		ImGui::IsKeyPressed(ImGuiKey_E) || ImGui::IsKeyPressed(ImGuiKey_Q);

	return ImGui::IsAnyMouseDown() && isAnyKeyDown;
}

bool ViewportCameraController::OnMouseWheel(float delta)
{
	//if (IsInWalkMode())
	//{
	//	m_walkSpeedScale += delta > 0.0f ? 1.0f : -1.0f;
	//	m_walkSpeedScale = std::clamp(m_walkSpeedScale, 0.1f, 10.0f);
	//}
	if (IsZooming())
	{
		float fixedDelta = delta > 0.0f ? 120.0f : -120.0f;
		delta = -fixedDelta * CalculateZoomScale() * 0.002f;
		if (fixedDelta > 0.0f)
		{
			float origLookAtDist = m_distanceFromLookAt;
			m_distanceFromLookAt += delta;
			delta = -fixedDelta * CalculateZoomScale() * 0.002f;
			m_distanceFromLookAt = origLookAtDist;
		}

		const float minWheelDelta = 1.5f;
		if (delta > -minWheelDelta && delta < minWheelDelta)
		{
			delta = delta < 0.0f ? -minWheelDelta : minWheelDelta;
		}
		m_distanceFromLookAt += delta;

		ControllerToCamera();
	}

	return true;
}

bool ViewportCameraController::OnMouseDown(float x, float y)
{
	if (IsInControl())
	{
		m_lastMousePoint.x() = x;
		m_lastMousePoint.y() = y;
		m_dragging = true;
		return true;
	}

	return false;
}

bool ViewportCameraController::OnMouseUp(float x, float y)
{
	if (m_dragging)
	{
		m_dragging = false;
		return true;
	}

	return false;
}

bool ViewportCameraController::OnMouseMove(float x, float y)
{
	if (!m_dragging || !IsInControl())
	{
		return false;
	}

	ImVec2 displaySize = ImGui::GetIO().DisplaySize;
	float dx = (x - m_lastMousePoint.x()) * (4.0f / displaySize.x);
	float dy = (y - m_lastMousePoint.y()) * (4.0f / displaySize.y);

	if (IsTracking())
	{
		m_elevation += dy * 4.0f;
		if (m_elevation > cd::Math::PI)
		{
			m_elevation -= cd::Math::TWO_PI;
		}
		else if (m_elevation < -cd::Math::PI)
		{
			m_elevation += cd::Math::TWO_PI;
		}

		m_azimuth += dx * 4.0f;
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
	else if (IsZooming())
	{
		float zoom = -dy - dx;
		m_distanceFromLookAt += zoom * CalculateZoomScale() * 0.7f;

		ControllerToCamera();
	}
	else if (IsPanning())
	{
		float scale = CalculateZoomScale() * 0.21f;
		float sign = scale > 0.0f ? 1.0f : -1.0f;
		m_lookAtPoint += sign * m_up * dy * scale;
		m_lookAtPoint += sign * m_lookAt.Cross(m_up) * dx * scale * GetMainCameraComponent()->GetAspect();

		ControllerToCamera();
	}

	m_lastMousePoint.x() = x;
	m_lastMousePoint.y() = y;
	return true;
}

bool ViewportCameraController::OnKeyDown()
{
	if (!IsInWalkMode())
	{
		return false;
	}

	cd::Direction direction(0.0f);
	if (ImGui::IsKeyPressed(ImGuiKey_W) || ImGui::IsKeyPressed(ImGuiKey_UpArrow))
	{
		direction -= cd::Vec3f(1.0f, 0.0f, 1.0f) * m_lookAt;
	}
	
	if (ImGui::IsKeyPressed(ImGuiKey_A) || ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
	{
		direction += cd::Vec3f(1.0f, 0.0f, 1.0f) * m_lookAt.Cross(m_up);
	}
	
	if (ImGui::IsKeyPressed(ImGuiKey_S) || ImGui::IsKeyPressed(ImGuiKey_DownArrow))
	{
		direction += cd::Vec3f(1.0f, 0.0f, 1.0f) * m_lookAt;
	}
	
	if (ImGui::IsKeyPressed(ImGuiKey_D) || ImGui::IsKeyPressed(ImGuiKey_RightArrow))
	{
		direction -= cd::Vec3f(1.0f, 0.0f, 1.0f) * m_lookAt.Cross(m_up);
	}
	
	if (ImGui::IsKeyPressed(ImGuiKey_E))
	{
		direction -= cd::Vec3f(0.0f, 1.0f, 0.0f) * m_up;
	}
	
	if (ImGui::IsKeyPressed(ImGuiKey_Q))
	{
		direction += cd::Vec3f(0.0f, 1.0f, 0.0f) * m_up;
	}

	direction.Normalize();

	cd::Point newEye = m_eye + direction * GetWalkSpeed();
	auto delta = m_eye - newEye;
	m_eye = newEye;
	m_lookAtPoint += delta;

	ControllerToCamera();

	return true;
}

void ViewportCameraController::CameraToController()
{
	m_eye = GetMainCameraTransform().GetTranslation();
	m_lookAt = CameraComponent::GetLookAt(GetMainCameraTransform());
	m_up = CameraComponent::GetUp(GetMainCameraTransform());
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

void ViewportCameraController::ControllerToCamera()
{
	cd::Vec3f eye = m_eye;
	cd::Vec3f lookAt = m_lookAt.Normalize();
	cd::Vec3f up = m_up.Normalize();

	float sinPhi = std::sin(m_elevation);
	float cosPhi = std::cos(m_elevation);
	float sinTheta = std::sin(m_azimuth);
	float cosTheta = std::cos(m_azimuth);

	lookAt = cd::Vec3f(-cosPhi * sinTheta, -sinPhi, -cosPhi * cosTheta);
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

	GetMainCameraComponent()->BuildViewMatrix(eye, lookAt, up);
	TransformComponent* pTransformComponent = GetMainCameraTransformComponent();
	pTransformComponent->GetTransform().SetTranslation(eye);
	
	cd::Vec3f rotationAxis = cd::Vec3f(0.0f, 0.0f, 1.0f).Cross(lookAt);
	float rotationAngle = std::acos(cd::Vec3f(0.0f, 0.0f, 1.0f).Dot(lookAt));
	pTransformComponent->GetTransform().SetRotation(cd::Quaternion::FromAxisAngle(rotationAxis, rotationAngle));
	pTransformComponent->Dirty();
	pTransformComponent->Build();
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
			m_walkSpeedScale = meshAABB.Size().Length() * 1.5f;
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