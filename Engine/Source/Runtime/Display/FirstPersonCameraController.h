#pragma once

// C/C++
#include <inttypes.h>

namespace engine
{

class FlybyCamera;

class FirstPersonCameraController {

public:
	FirstPersonCameraController() = delete;
	explicit FirstPersonCameraController(FlybyCamera* pCamera, const float mouse_sensitivity, const float movement_speed);
	~FirstPersonCameraController() = default;

	FirstPersonCameraController(const FirstPersonCameraController&) = delete;
	FirstPersonCameraController(FirstPersonCameraController&&) = delete;
	FirstPersonCameraController& operator=(const FirstPersonCameraController&) = delete;
	FirstPersonCameraController& operator=(FirstPersonCameraController&&) = delete;

	void Update(float dt);

private:
	FlybyCamera* m_pFlybyCamera;
	float m_mouseSensitivity;
	float m_movementSpeed;
};

}	// namespace engine