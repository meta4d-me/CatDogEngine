#include "FirstPersonCameraController.h"

#include "Display/CameraUtility.h"
#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "Math/Quaternion.hpp"
#include "Window/Input.h"

#include <cassert>

namespace engine
{

FirstPersonCameraController::FirstPersonCameraController(
	const SceneWorld* pSceneWorld,
	const float sensitivity, 
	const float movement_speed)
	: FirstPersonCameraController(pSceneWorld, sensitivity, sensitivity, movement_speed)
{
}

FirstPersonCameraController::FirstPersonCameraController(
	const SceneWorld* pSceneWorld,
	const float horizontal_sensitivity, 
	const float vertical_sensitivity, 
	const float movement_speed)
	: m_pSceneWorld(pSceneWorld)
	, m_horizontalSensitivity(horizontal_sensitivity)
	, m_verticalSensitivity(vertical_sensitivity)
	, m_movementSpeed(movement_speed)
{
	assert(pSceneWorld);
}

void FirstPersonCameraController::CameraToController()
{
	m_eye = GetMainCameraTransform().GetTranslation();
	m_lookAt = GetLookAt(GetMainCameraTransform());
	m_up = GetUp(GetMainCameraTransform());

	
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

void FirstPersonCameraController::ControllerToCamera()
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
	}
// There will be some operation to change eye and lookAt or up
	GetMainCameraComponent()->BuildView(eye, lookAt, up);
}

void FirstPersonCameraController::Update(float deltaTime)
{
	if (Input::Get().IsKeyPressed(KeyCode::w))
	{
		m_isMayaStyle = false;
		MoveForward(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::a))
	{
		m_isMayaStyle = false;
		MoveLeft(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::s))
	{
		m_isMayaStyle = false;
		MoveBackward(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::d))
	{
		m_isMayaStyle = false;
		MoveRight(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::e))
	{
		m_isMayaStyle = false;
		MoveUp(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::q))
	{
		m_isMayaStyle = false;
		MoveDown(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsMouseRBPressed())
	{
		m_isMayaStyle = false;
		PitchLocal(m_horizontalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
		Yaw(m_verticalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	}
	if (Input::Get().IsMouseLBPressed())
	{
		m_isMayaStyle = true;
		elevationChanging(m_horizontalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
		azimuthChanging(m_verticalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	}
	if (Input::Get().GetMouseScrollOffsetY())
	{
		float scaleDelta = Input::Get().GetMouseScrollOffsetY() * deltaTime;
		scaleDelta = scaleDelta > 0 ? scaleDelta : -scaleDelta;

		float delta = -scaleDelta * CalculateZoomScale() * 0.002f;
		if (scaleDelta > 0)
		{
			float origLookAtDist = m_distanceFromLookAt;
			m_distanceFromLookAt += delta;
			delta = -scaleDelta * CalculateZoomScale() * 0.0002f;
			m_distanceFromLookAt = origLookAtDist;
		}
		//minimum distance to travel in world space with one wheel "notch". If this is too
			// small, zooming can feel too slow as we get close to the look-at-point.
		const float min_wheel_delta = 1.5f;
		if (delta > -min_wheel_delta && delta < min_wheel_delta)
		{
			if (delta < 0.0f)
				delta = -min_wheel_delta;
			else
				delta = min_wheel_delta;
		}
		m_distanceFromLookAt += delta;
		ControllerToCamera();

	}
}

void FirstPersonCameraController::SetMovementSpeed(const float speed)
{
	m_movementSpeed = speed;
}

void FirstPersonCameraController::SetSensitivity(const float horizontal, const float verticle)
{
	m_horizontalSensitivity = horizontal;
	m_verticalSensitivity = verticle;
}

void FirstPersonCameraController::SetHorizontalSensitivity(const float sensitivity)
{
	m_horizontalSensitivity = sensitivity;
}

void FirstPersonCameraController::SetVerticalSensitivity(const float sensitivity)
{
	m_verticalSensitivity = sensitivity;
}

engine::CameraComponent* FirstPersonCameraController::GetMainCameraComponent() const
{
	return m_pSceneWorld->GetCameraComponent(m_pSceneWorld->GetMainCameraEntity());
}

cd::Transform FirstPersonCameraController::GetMainCameraTransform() 
{
	return m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity())->GetTransform();
}


void FirstPersonCameraController::MoveForward(float amount)
{
	m_eye = m_eye + m_lookAt * amount;
	ControllerToCamera();
}

void FirstPersonCameraController::MoveBackward(float amount)
{
	MoveForward(-amount);
}

void FirstPersonCameraController::MoveLeft(float amount)
{
	m_eye = m_eye + (m_lookAt.Cross(m_up) * amount);
	ControllerToCamera();
}

void FirstPersonCameraController::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void FirstPersonCameraController::MoveUp(float amount)
{
	m_eye = m_eye + m_up * amount;
	ControllerToCamera();
}

void FirstPersonCameraController::MoveDown(float amount)
{
	MoveUp(-amount);
}

void FirstPersonCameraController::Rotate(const cd::Vec3f& axis, float angleDegrees)
{
	cd::Quaternion rotation = cd::Quaternion::FromAxisAngle(axis, cd::Math::DegreeToRadian<float>(angleDegrees));
	m_lookAt = rotation * m_lookAt;
	m_up = rotation * m_up;
	ControllerToCamera();
}

void FirstPersonCameraController::Rotate(float x, float y, float z, float angleDegrees)
{
	Rotate(cd::Vec3f(x, y, z), angleDegrees);
}


void FirstPersonCameraController::Yaw(float angleDegrees)
{
	Rotate(0.0f, 1.0f, 0.0f, angleDegrees);
}

void FirstPersonCameraController::Pitch(float angleDegrees)
{
	Rotate(1.0f, 0.0f, 0.0f, angleDegrees);
}

void FirstPersonCameraController::Roll(float angleDegrees)
{
	Rotate(0.0f, 0.0f,1.0f, angleDegrees);
}

void FirstPersonCameraController::YawLocal(float angleDegrees)
{
	Rotate(m_up, angleDegrees);
}

void FirstPersonCameraController::PitchLocal(float angleDegrees)
{
	Rotate(m_up.Cross(m_lookAt), angleDegrees);
}

void FirstPersonCameraController::RollLocal(float angleDegrees)
{
	Rotate(m_lookAt, angleDegrees);
}

void FirstPersonCameraController::elevationChanging(float angleDegrees)
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

void FirstPersonCameraController::azimuthChanging(float angleDegrees)
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

}	// namespace engine