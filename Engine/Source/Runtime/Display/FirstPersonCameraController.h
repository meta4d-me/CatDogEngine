#pragma once

// C/C++
#include <inttypes.h>

namespace engine
{

class FlybyCamera;

class FirstPersonCameraController {

public:
	FirstPersonCameraController() = delete;
	explicit FirstPersonCameraController(FlybyCamera* pCamera, const float sensitivity, const float movement_speed);
	explicit FirstPersonCameraController(FlybyCamera* pCamera, const float horizontal_sensitivity, const float vertical_sensitivity, const float movement_speed);
	~FirstPersonCameraController() = default;

	FirstPersonCameraController(const FirstPersonCameraController&) = delete;
	FirstPersonCameraController(FirstPersonCameraController&&) = delete;
	FirstPersonCameraController& operator=(const FirstPersonCameraController&) = delete;
	FirstPersonCameraController& operator=(FirstPersonCameraController&&) = delete;

	void Update(float dt);

	void setMovementSpeed(const float speed);
	void setSensitivity(const float horizontal, const float verticle);
	void setHorizontalSensitivity(const float sensitivity);
	void setVerticalSensitivity(const float sensitivity);

private:
	FlybyCamera* m_pFlybyCamera;
	float m_horizontalSensitivity;
	float m_verticalSensitivity;
	float m_movementSpeed;
};

}	// namespace engine