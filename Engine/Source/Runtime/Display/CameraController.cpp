#include "CameraController.h"

#include "Display/CameraUtility.h"
#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "Math/Quaternion.hpp"
#include "Window/Input.h"

#include <cassert>
#include <cmath>

namespace engine
{

CameraController::CameraController(
	const SceneWorld* pSceneWorld,
	const float sensitivity, 
	const float movement_speed)
	: CameraController(pSceneWorld, sensitivity, sensitivity, movement_speed)
{
}

CameraController::CameraController(
	const SceneWorld* pSceneWorld,
	const float horizontal_sensitivity, 
	const float vertical_sensitivity, 
	const float movement_speed)
	: m_pSceneWorld(pSceneWorld)
	, m_horizontalSensitivity(horizontal_sensitivity)
	, m_verticalSensitivity(vertical_sensitivity)
	, m_movementSpeed(movement_speed)
	, m_initialMovemenSpeed(movement_speed)
{
	assert(pSceneWorld);
}

void CameraController::CameraToController()
{
	m_eye = GetMainCameraTransform().GetTranslation();
	m_lookAt = GetLookAt(GetMainCameraTransform());
	m_up = GetUp(GetMainCameraTransform());
}

void CameraController::ControllerToCamera()
{
	cd::Vec3f eye = m_eye;
	cd::Vec3f lookAt = m_lookAt;
	cd::Vec3f up = m_up;

	if (m_isMayaStyle)
	{
		float sinPhi = std::sin(m_elevation);
		float cosPhi = std::cos(m_elevation);
		float sinTheta = std::sin(m_azimuth);
		float cosTheta = std::cos(m_azimuth);

		lookAt = cd::Vec3f(-cosPhi * sinTheta, -sinPhi, -cosPhi * cosTheta);
		cd::Vec3f cross = cd::Vec3f(cosTheta, 0, -sinTheta);
		up =  cross.Cross(lookAt);
		float lookAtOffset = 0;
		if (m_distanceFromLookAt < m_dollyThreshold) 
			lookAtOffset = m_distanceFromLookAt - m_dollyThreshold;

		float eyeOffset = m_distanceFromLookAt;
		eye = m_lookAtPoint - (lookAt * eyeOffset);

		//synchronize view to fpscamera
		m_eye = eye;
		m_lookAt = lookAt;
		m_up = up;
	}
	GetMainCameraComponent()->BuildView(eye, lookAt, up);
	GetMainCameraTransformComponent()->GetTransform().SetTranslation(eye);
	//maybe i can wrap this?
	cd::Vec3f rotAxis = cd::Vec3f(0.0,0.0,1.0).Cross(lookAt.Normalize()).Normalize();
	float rotAngle = std::acos(cd::Vec3f(0.0, 0.0, 1.0).Dot(lookAt.Normalize()));
	GetMainCameraTransformComponent()->GetTransform().SetRotation(cd::Quaternion::FromAxisAngle(rotAxis, rotAngle));
	GetMainCameraTransformComponent()->Build();
}

void CameraController::Update(float deltaTime)
{

	if (Input::Get().GetMouseScrollOffsetY() && (Input::Get().IsMouseLBPressed() || Input::Get().IsMouseRBPressed()) && !Input::Get().IsKeyPressed(KeyCode::z))
	{
		m_mouseScroll += Input::Get().GetMouseScrollOffsetY() / 10;
		m_mouseScroll = std::clamp(m_mouseScroll, -3.0f, 2.5f);
		float speedRate = std::pow(2.0f, m_mouseScroll);
		m_movementSpeed = speedRate * m_initialMovemenSpeed;
	}
	if (Input::Get().IsKeyPressed(KeyCode::w) && (Input::Get().IsMouseLBPressed() || Input::Get().IsMouseRBPressed()) && !Input::Get().IsKeyPressed(KeyCode::z))
	{
		m_isMayaStyle = false;
		MoveForward(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::a) && (Input::Get().IsMouseLBPressed() || Input::Get().IsMouseRBPressed()) && !Input::Get().IsKeyPressed(KeyCode::z))
	{
		m_isMayaStyle = false;
		MoveLeft(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::s) && (Input::Get().IsMouseLBPressed() || Input::Get().IsMouseRBPressed()) && !Input::Get().IsKeyPressed(KeyCode::z))
	{
		m_isMayaStyle = false;
		MoveBackward(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::d) && (Input::Get().IsMouseLBPressed() || Input::Get().IsMouseRBPressed()) && !Input::Get().IsKeyPressed(KeyCode::z))
	{
		m_isMayaStyle = false;
		MoveRight(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::e) && (Input::Get().IsMouseLBPressed() || Input::Get().IsMouseRBPressed()) && !Input::Get().IsKeyPressed(KeyCode::z))
	{
		m_isMayaStyle = false;
		MoveUp(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::q) && (Input::Get().IsMouseLBPressed() || Input::Get().IsMouseRBPressed()) && !Input::Get().IsKeyPressed(KeyCode::z))
	{
		m_isMayaStyle = false;
		MoveDown(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsMouseLBPressed() && !Input::Get().IsKeyPressed(KeyCode::z)&& !m_isFocusing)
	{
		m_isMayaStyle = false;
		Yaw(m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	}
	if (Input::Get().IsMouseRBPressed() && !Input::Get().IsKeyPressed(KeyCode::z))
	{
		m_isMayaStyle = false;
		PitchLocal(m_verticalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
		Yaw(m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	}
	if (Input::Get().IsMouseLBPressed() && Input::Get().IsKeyPressed(KeyCode::z) && !m_isFocusing)
	{
		m_isMayaStyle = true;
		SynchronizeMayaCamera();//dont need do this in every update. it'only do once befor using mayastyle
		ElevationChanging(m_verticalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
		AzimuthChanging(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	}
	if (Input::Get().IsMouseRBPressed() && Input::Get().IsKeyPressed(KeyCode::z))
	{
		float scaleDelta = Input::Get().GetMousePositionOffsetX() * deltaTime * 10;
		m_distanceFromLookAt -= scaleDelta;
		m_eye = m_eye + m_lookAt * scaleDelta;
		ControllerToCamera();
	}

	if (Input::Get().GetMouseScrollOffsetY() && Input::Get().IsKeyPressed(KeyCode::z))
	{
		float scaleDelta = Input::Get().GetMouseScrollOffsetY() * deltaTime * 500;
	
		m_distanceFromLookAt -= scaleDelta;
		m_eye = m_eye + m_lookAt * scaleDelta;
		ControllerToCamera();
	}
	if (Input::Get().Get().IsKeyPressed(KeyCode::z))
	{
		SynchronizeMayaCamera();
	}
	Focusing();
}

void CameraController::SetMovementSpeed(const float speed)
{
	m_movementSpeed = speed;
}

void CameraController::SetSensitivity(const float horizontal, const float verticle)
{
	m_horizontalSensitivity = horizontal;
	m_verticalSensitivity = verticle;
}

void CameraController::SetHorizontalSensitivity(const float sensitivity)
{
	m_horizontalSensitivity = sensitivity;
}

void CameraController::SetVerticalSensitivity(const float sensitivity)
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

cd::Transform CameraController::GetMainCameraTransform() 
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
	m_eye = m_eye + (m_lookAt.Cross(m_up) * amount);
	ControllerToCamera();
}

void CameraController::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void CameraController::MoveUp(float amount)
{
	m_eye = m_eye + m_up * amount;
	ControllerToCamera();
}

void CameraController::MoveDown(float amount)
{
	MoveUp(-amount);
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
	Rotate(0.0f, 0.0f,1.0f, angleDegrees);
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

void CameraController::SynchronizeMayaCamera()
{
	m_lookAtPoint = m_lookAt * m_distanceFromLookAt + m_eye;
	m_elevation = std::asin(-m_lookAt.y());
	if (m_up.y() < 0)
	{
		if (m_lookAt.y() > 0)
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

void CameraController::CameraFocus(const cd::AABB& aabb)
{
	if (aabb.IsEmpty())
	{
		return;
	}
	m_isFocusing = true;
	m_distanceFromLookAt = (aabb.Max() - aabb.Center()).Length() * 3;
	m_eyeDestination = aabb.Center() - m_lookAt * m_distanceFromLookAt;
}

void CameraController::Focusing()
{
	if (m_isFocusing)
	{
		cd::Vec3f eyeMoveDir = (m_eye - m_eyeDestination).Normalize();
		float stepDistance = (m_eye - m_eyeDestination).Length() / 5;
		m_eye = m_eye - eyeMoveDir * stepDistance;
		SynchronizeMayaCamera();
		CD_INFO("Focusing");
		ControllerToCamera();
		if ((m_eye - m_eyeDestination).Length()< 0.1f)
		{
			m_isFocusing = false;
		}
			
	}
}

}	// namespace engine