#include "FirstPersonCameraController.h"

#include "FlybyCamera.h"

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
	, m_mousePrevX(0)
	, m_mousePrevY(0)
	, m_mouseSensitivity(mouse_sensitivity)
	, m_movementSpeed(movement_speed)
{}

void FirstPersonCameraController::Update(const float dt)
{
	if (!m_pFlybyCamera)
	{
		return;
	}

	if (m_isWKeyDown)
	{
		m_pFlybyCamera->Translate(0.0f, 0.0f, m_movementSpeed * dt);
	}
	if (m_isAKeyDown)
	{
		m_pFlybyCamera->Translate(-m_movementSpeed * dt, 0.0f, 0.0f);
	}
	if (m_isSKeyDown)
	{
		m_pFlybyCamera->Translate(0.0f, 0.0f, -m_movementSpeed * dt);
	}
	if (m_isDKeyDown)
	{
		m_pFlybyCamera->Translate(m_movementSpeed * dt, 0.0f, 0.0f);
	}
	if (m_isQKeyDown)
	{
		m_pFlybyCamera->Translate(0.0f, m_movementSpeed * dt, 0.0f);
	}
	if (m_isEKeyDown)
	{
		m_pFlybyCamera->Translate(0.0f, -m_movementSpeed * dt, 0.0f);
	}
	if (m_isRightMouseDown)
	{
		const float mouse_dx = static_cast<float>(m_mouseX) - m_mousePrevX;
		const float mouse_dy = static_cast<float>(m_mouseY) - m_mousePrevY;
		m_pFlybyCamera->PitchLocal(m_mouseSensitivity * mouse_dx * dt);
		m_pFlybyCamera->Yaw(m_mouseSensitivity * mouse_dy * dt);
	}
}

void FirstPersonCameraController::OnKeyPress(const SDL_Keycode& keyCode, const uint16_t mods)
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

void FirstPersonCameraController::OnKeyRelease(const SDL_Keycode& keyCode, const uint16_t mods)
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

void FirstPersonCameraController::OnMousePress(const uint8_t button, const uint8_t clicks)
{
	switch (button)
	{
	case SDL_BUTTON_RIGHT:
		m_isRightMouseDown = true;
		break;
	default:
		break;
	}
}

void FirstPersonCameraController::OnMouseRelease(const uint8_t button, const uint8_t clicks)
{
	switch (button)
	{
	case SDL_BUTTON_RIGHT:
		m_isRightMouseDown = false;
		break;
	default:
		break;
	}
}

void FirstPersonCameraController::SetMousePosition(const int32_t win_x, const int32_t win_y)
{
	m_mousePrevX = m_mouseX;
	m_mousePrevY = m_mouseY;
	m_mouseX = win_x;
	m_mouseY = win_y;
}

}	// namespace engine