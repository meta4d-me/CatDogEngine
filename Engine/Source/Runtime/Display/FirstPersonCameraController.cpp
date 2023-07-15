#include "FirstPersonCameraController.h"

#include "Display/CameraUtility.h"
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

cd::Transform FirstPersonCameraController::GetMainCameraTransform() 
{
	return m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity())->GetTransform();
}


void FirstPersonCameraController::MoveForward(float amount)
{
	if (TransformComponent* pTranformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		pTranformComponent->GetTransform().SetTranslation(pTranformComponent->GetTransform().GetTranslation() + GetLookAt(pTranformComponent->GetTransform()) * amount);
		GetMainCameraComponent()->ViewDirty();
	}
}

void FirstPersonCameraController::MoveBackward(float amount)
{
	MoveForward(-amount);
}

void FirstPersonCameraController::MoveLeft(float amount)
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		pTransformComponent->GetTransform().SetTranslation(pTransformComponent->GetTransform().GetTranslation() - GetCross(pTransformComponent->GetTransform()) * amount);
		GetMainCameraComponent()->ViewDirty();
	}
}

void FirstPersonCameraController::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void FirstPersonCameraController::MoveUp(float amount)
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		pTransformComponent->GetTransform().SetTranslation(pTransformComponent->GetTransform().GetTranslation() + GetUp(pTransformComponent->GetTransform()) * amount);
		GetMainCameraComponent()->ViewDirty();
	}
}

void FirstPersonCameraController::MoveDown(float amount)
{
	MoveUp(-amount);
}

void FirstPersonCameraController::Rotate(const cd::Vec3f& axis, float angleDegrees)
{
	cd::Quaternion rotation = cd::Quaternion::FromAxisAngle(axis, cd::Math::DegreeToRadian<float>(angleDegrees));
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{	
		SetLookAt(rotation * GetLookAt(pTransformComponent->GetTransform()), pTransformComponent->GetTransform());
		SetUp(rotation * GetUp(pTransformComponent->GetTransform()), pTransformComponent->GetTransform());
		GetMainCameraComponent()->ViewDirty();
		//cd::Vec3f lookAt = GetLookAt(pTransformComponent->GetTransform());
		//cd::Vec3f up = GetUp(pTransformComponent->GetTransform());
		//cd::Vec3f eye = pTransformComponent->GetTransform().GetTranslation();
		//cd::Matrix4x4 viewMatrix = cd::Matrix4x4::LookAt<cd::Handedness::Left>(eye, eye + lookAt, up);
	}
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
	Rotate(0.0f, 0.0f,1.0f, angleDegrees);
}

void FirstPersonCameraController::YawLocal(float angleDegrees)
{
	Rotate(GetUp(m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity())->GetTransform()), angleDegrees);
}

void FirstPersonCameraController::PitchLocal(float angleDegrees)
{
	Rotate(GetCross(m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity())->GetTransform()), angleDegrees);
}

void FirstPersonCameraController::RollLocal(float angleDegrees)
{
	Rotate(GetLookAt(m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity())->GetTransform()), angleDegrees);
}

}	// namespace engine