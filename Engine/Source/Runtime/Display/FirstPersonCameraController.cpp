#include "FirstPersonCameraController.h"

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

void FirstPersonCameraController::Update(float deltaTime)
{
	if (Input::Get().IsKeyPressed(KeyCode::w))
	{
		MoveForward(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::a))
	{
		MoveLeft(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::s))
	{
		MoveBackward(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::d))
	{
		MoveRight(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::e))
	{
		MoveUp(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsKeyPressed(KeyCode::q))
	{
		MoveDown(m_movementSpeed * deltaTime);
	}

	if (Input::Get().IsMouseRBPressed())
	{
		PitchLocal(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
		Yaw(m_verticalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
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

void FirstPersonCameraController::MoveForward(float amount)
{
	engine::CameraComponent* pCameraComponent = GetMainCameraComponent();
	//pCameraComponent->SetEye(pCameraComponent->GetEye() + pCameraComponent->GetLookAt() * amount);
}

void FirstPersonCameraController::MoveBackward(float amount)
{
	MoveForward(-amount);
}

void FirstPersonCameraController::MoveLeft(float amount)
{
	engine::CameraComponent* pCameraComponent = GetMainCameraComponent();
	//pCameraComponent->SetEye(pCameraComponent->GetEye() - pCameraComponent->GetCross() * amount);
}

void FirstPersonCameraController::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void FirstPersonCameraController::MoveUp(float amount)
{
	engine::CameraComponent* pCameraComponent = GetMainCameraComponent();
	//pCameraComponent->SetEye(pCameraComponent->GetEye() + pCameraComponent->GetUp() * amount);
}

void FirstPersonCameraController::MoveDown(float amount)
{
	MoveUp(-amount);
}

void FirstPersonCameraController::Rotate(const cd::Vec3f& axis, float angleDegrees)
{
	engine::CameraComponent* pCameraComponent = GetMainCameraComponent();
	cd::Quaternion rotation = cd::Quaternion::FromAxisAngle(axis, cd::Math::DegreeToRadian<float>(angleDegrees));
	//pCameraComponent->SetLookAt(rotation * pCameraComponent->GetLookAt());
	//pCameraComponent->SetUp(rotation * pCameraComponent->GetUp());
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
	//Rotate(GetMainCameraComponent()->GetUp(), angleDegrees);
}

void FirstPersonCameraController::PitchLocal(float angleDegrees)
{
	//Rotate(GetMainCameraComponent()->GetCross(), angleDegrees);
}

void FirstPersonCameraController::RollLocal(float angleDegrees)
{
	//Rotate(GetMainCameraComponent()->GetLookAt(), angleDegrees);
}

}	// namespace engine