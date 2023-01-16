#include "FirstPersonCameraController.h"

#include "FlybyCamera.h"
#include "Window/Input.h"

#include <cassert>

namespace engine
{

FirstPersonCameraController::FirstPersonCameraController(FlybyCamera* pCamera, const float mouse_sensitivity, const float movement_speed)
	: m_pFlybyCamera(pCamera)
	, m_mouseSensitivity(mouse_sensitivity)
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

	if (Input::Get().IsKeyPressed(KeyCode::q))
	{
		m_pFlybyCamera->MoveUp(m_movementSpeed * dt);
	}

	if (Input::Get().IsKeyPressed(KeyCode::e))
	{
		m_pFlybyCamera->MoveDown(m_movementSpeed * dt);
	}

	if (Input::Get().IsMouseRBPressed())
	{
		m_pFlybyCamera->PitchLocal(m_mouseSensitivity * Input::Get().GetMousePositionOffsetY() * dt);
		m_pFlybyCamera->Yaw(-m_mouseSensitivity * Input::Get().GetMousePositionOffsetX() * dt);
	}
}

}	// namespace engine