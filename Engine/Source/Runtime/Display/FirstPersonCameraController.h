#pragma once

// C/C++
#include <inttypes.h>

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

	void Update(float dt);
	void OnKeyPress(int32_t keyCode, uint16_t mods);
	void OnKeyRelease(int32_t keyCode, uint16_t mods);
	void OnMouseRBPress();
	void OnMouseRBRelease();
	void SetMousePosition(int32_t x, int32_t y);

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