#include "FirstPersonCameraController.h"

#include "FlybyCamera.h"
#include "Window/Input.h"

#include <cassert>

namespace engine
{

FirstPersonCameraController::FirstPersonCameraController(
	FlybyCamera* pCamera, 
	const float sensitivity, 
	const float movement_speed)
	: FirstPersonCameraController(pCamera, sensitivity, sensitivity, movement_speed)
{}

FirstPersonCameraController::FirstPersonCameraController(
	FlybyCamera* pCamera, 
	const float horizontal_sensitivity, 
	const float vertical_sensitivity, 
	const float movement_speed)
	: m_pFlybyCamera(pCamera)
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
		m_pFlybyCamera->MoveForward(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::a))
	{
		m_pFlybyCamera->MoveLeft(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::s))
	{
		m_pFlybyCamera->MoveBackward(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::d))
	{
		m_pFlybyCamera->MoveRight(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::e))
	{
		m_pFlybyCamera->MoveUp(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::q))
	{
		m_pFlybyCamera->MoveDown(m_movementSpeed * dt);
	}

	if (Input::Get().IsMouseRBPressed())
	{
		m_pFlybyCamera->PitchLocal(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetY() * dt);
		m_pFlybyCamera->Yaw(m_verticalSensitivity * Input::Get().GetMousePositionOffsetX() * dt);
	}
}

void FirstPersonCameraController::setMovementSpeed(const float speed)
{
	m_movementSpeed = speed;
}

void FirstPersonCameraController::setSensitivity(const float horizontal, const float verticle)
{
	m_horizontalSensitivity = horizontal;
	m_verticalSensitivity = verticle;
}

void FirstPersonCameraController::setHorizontalSensitivity(const float sensitivity)
{
	m_horizontalSensitivity = sensitivity;
}

void FirstPersonCameraController::setVerticalSensitivity(const float sensitivity)
{
	m_verticalSensitivity = sensitivity;
}

}	// namespace engine