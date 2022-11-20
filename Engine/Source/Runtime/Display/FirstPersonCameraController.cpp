#include "FirstPersonCameraController.h"

#include "FlybyCamera.h"

#include <sdl.h>

namespace engine
{

FirstPersonCameraController::FirstPersonCameraController(FlybyCamera* camera, const float mouse_sensitivity, const float movement_speed)
	: m_pFlybyCamera(camera)
	, m_isWKeyDown(false)
	, m_isAKeyDown(false)
	, m_isSKeyDown(false)
	, m_isDKeyDown(false)
	, m_isQKeyDown(false)
	, m_isEKeyDown(false)
	, m_isRightMouseDown(false)
	, m_mouseX(0)
	, m_mouseY(0)
	, m_mouseSensitivity(mouse_sensitivity)
	, m_movementSpeed(movement_speed)
{}

void FirstPersonCameraController::Update(float dt)
{
	if (!m_pFlybyCamera)
	{
		return;
	}

	if (m_isWKeyDown)
	{
		m_pFlybyCamera->MoveForward(m_movementSpeed * dt);
	}
	if (m_isAKeyDown)
	{
		m_pFlybyCamera->MoveLeft(m_movementSpeed * dt);
	}
	if (m_isSKeyDown)
	{
		m_pFlybyCamera->MoveBackward(m_movementSpeed * dt);
	}
	if (m_isDKeyDown)
	{
		m_pFlybyCamera->MoveRight(m_movementSpeed * dt);
	}
	if (m_isQKeyDown)
	{
		m_pFlybyCamera->MoveUp(m_movementSpeed * dt);
	}
	if (m_isEKeyDown)
	{
		m_pFlybyCamera->MoveDown(m_movementSpeed * dt);
	}
	if (m_isRightMouseDown)
	{
		m_pFlybyCamera->PitchLocal(m_mouseSensitivity * m_mouseY * dt);
		m_pFlybyCamera->Yaw(-m_mouseSensitivity * m_mouseX * dt);
	}
}

void FirstPersonCameraController::OnKeyPress(int32_t keyCode, uint16_t mods)
{
	switch (keyCode)
	{
	case SDL_KeyCode::SDLK_w:
		m_isWKeyDown = true;
		break;
	case SDL_KeyCode::SDLK_a:
		m_isAKeyDown = true;
		break;
	case SDL_KeyCode::SDLK_s:
		m_isSKeyDown = true;
		break;
	case SDL_KeyCode::SDLK_d:
		m_isDKeyDown = true;
		break;
	case SDL_KeyCode::SDLK_e:
		m_isEKeyDown = true;
		break;
	case SDL_KeyCode::SDLK_q:
		m_isQKeyDown = true;
		break;
	default:
		break;
	}
}

void FirstPersonCameraController::OnKeyRelease(int32_t keyCode, uint16_t mods)
{
	switch (keyCode)
	{
	case SDL_KeyCode::SDLK_w:
		m_isWKeyDown = false;
		break;
	case SDL_KeyCode::SDLK_a:
		m_isAKeyDown = false;
		break;
	case SDL_KeyCode::SDLK_s:
		m_isSKeyDown = false;
		break;
	case SDL_KeyCode::SDLK_d:
		m_isDKeyDown = false;
		break;
	case SDL_KeyCode::SDLK_e:
		m_isEKeyDown = false;
		break;
	case SDL_KeyCode::SDLK_q:
		m_isQKeyDown = false;
		break;
	default:
		break;
	}
}

void FirstPersonCameraController::OnMouseRBPress()
{
	m_isRightMouseDown = true;
}

void FirstPersonCameraController::OnMouseRBRelease()
{
	m_isRightMouseDown = false;
}

void FirstPersonCameraController::SetMousePosition(int32_t x, int32_t y)
{
	m_mouseX = x;
	m_mouseY = y;
}

}	// namespace engine