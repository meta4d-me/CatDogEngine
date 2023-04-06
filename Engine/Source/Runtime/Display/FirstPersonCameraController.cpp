#include "FirstPersonCameraController.h"

#include "ECWorld/CameraComponent.h"
#include "Math/Quaternion.hpp"
#include "Window/Input.h"

#include <cassert>

namespace engine
{

FirstPersonCameraController::FirstPersonCameraController(
	CameraComponent* pCamera,
	const float sensitivity, 
	const float movement_speed)
	: FirstPersonCameraController(pCamera, sensitivity, sensitivity, movement_speed)
{}

FirstPersonCameraController::FirstPersonCameraController(
	CameraComponent* pCamera,
	const float horizontal_sensitivity, 
	const float vertical_sensitivity, 
	const float movement_speed)
	: m_pCameraComponent(pCamera)
	, m_horizontalSensitivity(horizontal_sensitivity)
	, m_verticalSensitivity(vertical_sensitivity)
	, m_movementSpeed(movement_speed)
{
	assert(pCamera && "pCamera is nullptr");
}

void FirstPersonCameraController::Update(float dt)
{
	if (Input::Get().IsKeyPressed(KeyCode::w))
	{
		MoveForward(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::a))
	{
		MoveLeft(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::s))
	{
		MoveBackward(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::d))
	{
		MoveRight(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::e))
	{
		MoveUp(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::q))
	{
		MoveDown(m_movementSpeed * dt);
	}

	if (Input::Get().IsMouseRBPressed())
	{
		PitchLocal(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetY() * dt);
		Yaw(m_verticalSensitivity * Input::Get().GetMousePositionOffsetX() * dt);
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

void FirstPersonCameraController::MoveForward(float amount)
{
	m_pCameraComponent->SetEye(m_pCameraComponent->GetEye() + m_pCameraComponent->GetLookAt() * amount);
}

void FirstPersonCameraController::MoveBackward(float amount)
{
	MoveForward(-amount);
}

void FirstPersonCameraController::MoveLeft(float amount)
{
	m_pCameraComponent->SetEye(m_pCameraComponent->GetEye() - m_pCameraComponent->GetCross() * amount);
}

void FirstPersonCameraController::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void FirstPersonCameraController::MoveUp(float amount)
{
	m_pCameraComponent->SetEye(m_pCameraComponent->GetEye() + m_pCameraComponent->GetUp() * amount);
}

void FirstPersonCameraController::MoveDown(float amount)
{
	MoveUp(-amount);
}

void FirstPersonCameraController::Rotate(const cd::Vec3f& axis, float angleDegrees)
{
	cd::Quaternion rotation = cd::Quaternion::FromAxisAngle(axis, cd::Math::DegreeToRadian<float>(angleDegrees));
	m_pCameraComponent->SetLookAt(rotation * m_pCameraComponent->GetLookAt());
	m_pCameraComponent->SetUp(rotation * m_pCameraComponent->GetUp());
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
	Rotate(0.0f, 0.0f, 1.0f, angleDegrees);
}

void FirstPersonCameraController::YawLocal(float angleDegrees)
{
	Rotate(m_pCameraComponent->GetUp(), angleDegrees);
}

void FirstPersonCameraController::PitchLocal(float angleDegrees)
{
	Rotate(m_pCameraComponent->GetCross(), angleDegrees);
}

void FirstPersonCameraController::RollLocal(float angleDegrees)
{
	Rotate(m_pCameraComponent->GetLookAt(), angleDegrees);
}

}	// namespace engine