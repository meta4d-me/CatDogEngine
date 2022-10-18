#pragma once

// C/C++
#include <inttypes.h>

using SDL_Keycode = int32_t;

namespace engine
{

class FlybyCamera;

class FirstPersonCameraController {

public:
	FirstPersonCameraController() = delete;
	explicit FirstPersonCameraController(FlybyCamera* camera, const float mouse_sensitivity, const float movement_speed);
	~FirstPersonCameraController() = default;

	FirstPersonCameraController(const FirstPersonCameraController&) = delete;
	FirstPersonCameraController(FirstPersonCameraController&&) = delete;
	FirstPersonCameraController& operator=(const FirstPersonCameraController&) = delete;
	FirstPersonCameraController& operator=(FirstPersonCameraController&&) = delete;

	void Update(const float dt);
	void OnKeyPress(const SDL_Keycode& keyCode, const uint16_t mods);
	void OnKeyRelease(const SDL_Keycode& keyCode, const uint16_t mods);
	void OnMousePress(const uint8_t button, const uint8_t clicks);
	void OnMouseRelease(const uint8_t button, const uint8_t clicks);
	void SetMousePosition(const int32_t win_x, const int32_t win_y);

private:
	FlybyCamera* m_pFlybyCamera;
	bool m_isWKeyDown;
	bool m_isAKeyDown;
	bool m_isSKeyDown;
	bool m_isDKeyDown;
	bool m_isQKeyDown;
	bool m_isEKeyDown;
	bool m_isRightMouseDown;
	int32_t m_mouseX;
	int32_t m_mouseY;
	float m_mouseSensitivity;
	float m_movementSpeed;

};

}	// namespace engine