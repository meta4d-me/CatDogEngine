#include"IndustryCameraController.h"

namespace engine
{
IndustryCameraController::IndustryCameraController(
	const SceneWorld* pSceneWorld,
	const float sensitivity,
	const float movement_speed)
	: IndustryCameraController(pSceneWorld, sensitivity, sensitivity, movement_speed)
{
}

IndustryCameraController::IndustryCameraController(
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

void IndustryCameraController::Update(float deltaTime)
{
	if (Input::Get().IsKeyPressed(KeyCode::i))
	{
		if (Input::Get().IsMouseLBPressed())
		{
			RotatedVertical(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
			RotateHorizon(-m_verticalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);

		}
		if (Input::Get().IsMouseRBPressed())
		{
			ChangeDistance(-m_movementSpeed * Input::Get().GetMousePositionOffsetX() * deltaTime / 10000.0f);
			ChangeDistance(-m_movementSpeed * Input::Get().GetMousePositionOffsetY() * deltaTime / 10000.0f);
		}
	}
	if (Input::Get().IsKeyPressed(KeyCode::w))
	{
		MoveForward(m_movementSpeed * deltaTime);
	}
	if (Input::Get().IsKeyPressed(KeyCode::s))
	{
		MoveBackward(m_movementSpeed * deltaTime);
	}
	if (Input::Get().IsKeyPressed(KeyCode::a))
	{
		MoveLeft(m_movementSpeed * deltaTime);
	}
	if (Input::Get().IsKeyPressed(KeyCode::d))
	{
		MoveRight(m_movementSpeed * deltaTime);
	}
	if (Input::Get().IsMouseRBPressed())
	{
		Pitch(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
		Yaw(m_verticalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
	}
}
CameraComponent* IndustryCameraController::GetMainCameraComponent()const
{
	return m_pSceneWorld->GetCameraComponent(m_pSceneWorld->GetMainCameraEntity());
}

TransformComponent* IndustryCameraController::GetTransformComponent()const
{
	return m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity());
}

void IndustryCameraController::RotateHorizon(float amount)
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		m_cameraPosition = pTransformComponent->GetTransform().GetTranslation();
		cd::Transform transform = pTransformComponent->GetTransform();
		m_orbitCenter = cd::Vec3f(0, 0, 0);
		cd::Vec3f pc = m_cameraPosition - m_orbitCenter;

		m_horizontalAngle = amount * m_horizontalSensitivity;
		float horizonX = pc.x() * std::cos(cd::Math::DegreeToRadian(m_horizontalAngle)) - pc.z() * std::sin(cd::Math::DegreeToRadian(m_horizontalAngle));
		float horizonZ = pc.x() * std::sin(cd::Math::DegreeToRadian(m_horizontalAngle)) + pc.z() * std::cos(cd::Math::DegreeToRadian(m_horizontalAngle));
		cd::Vec3f horizonPosition = cd::Vec3f(horizonX, pc.y(), horizonZ) + m_orbitCenter;
		pTransformComponent->GetTransform().SetTranslation(horizonPosition);
		cd::Quaternion horizonRotation = cd::Quaternion::FromAxisAngle(cd::Vec3f(0, -1, 0), cd::Math::DegreeToRadian(m_horizontalAngle));
		pTransformComponent->GetTransform().SetRotation(horizonRotation * transform.GetRotation());
	}	
}

void IndustryCameraController::RotatedVertical(float amount)
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		m_cameraPosition = pTransformComponent->GetTransform().GetTranslation();
		cd::Transform transform = pTransformComponent->GetTransform();
		m_orbitCenter = cd::Vec3f(0, 0, 0);
		cd::Vec3f pc = m_cameraPosition - m_orbitCenter;

		m_verticalAngle = amount * m_verticalSensitivity;
		cd::Matrix3x3 Rotation = pTransformComponent->GetTransform().GetRotation().ToMatrix3x3();
		cd::Vec3f cameraRight(Rotation.Data(0), Rotation.Data(3), Rotation.Data(6));
		cd::Vec3f v = cameraRight;

		float sin = std::sin(cd::Math::DegreeToRadian(m_verticalAngle));
		float cos = std::cos(cd::Math::DegreeToRadian(m_verticalAngle));

		float new_x = (v.x() * v.x() * (1 - cos) + cos) * pc.x() + (v.x() * v.y() * (1 - cos) - v.z() * sin) * pc.y() + (v.x() * v.z() * (1 - cos) + v.y() * sin) * pc.z();
		float new_y = (v.x() * v.y() * (1 - cos) + v.z() * sin) * pc.x() + (v.y() * v.y() * (1 - cos) + cos) * pc.y() + (v.x() * v.z() * (1 - cos) - v.x() * sin) * pc.z();
		float new_z = (v.x() * v.z() * (1 - cos) * v.y() * sin) * pc.x() + (v.y() * v.z() * (1 - cos) + v.x() * sin) * pc.y() + (v.z() * v.z() * (1 - cos) + cos) * pc.z();

		cd::Vec3f verticalPosition(new_x, new_y, new_z);
		pTransformComponent->GetTransform().SetTranslation(verticalPosition);

		cd::Quaternion verticalRotation = cd::Quaternion::FromAxisAngle(v, cd::Math::DegreeToRadian(m_verticalAngle));
		pTransformComponent->GetTransform().SetRotation(verticalRotation * transform.GetRotation());
	}
}

void IndustryCameraController::ChangeDistance(float amount) 
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		m_distanceScale = amount * m_movementSpeed + 1;
		pTransformComponent->GetTransform().SetTranslation(pTransformComponent->GetTransform().GetTranslation() * m_distanceScale);
	}
}

void IndustryCameraController::MoveForward(float amount)
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		cd::Matrix4x4 rotMatrix = pTransformComponent->GetTransform().GetRotation().ToMatrix4x4();
		cd::Vec3f transVec = pTransformComponent->GetTransform().GetTranslation();
		cd::Vec3f cameraFront(-rotMatrix.Data(8), -rotMatrix.Data(9), rotMatrix.Data(10));
		cameraFront = cameraFront * amount + transVec;
		pTransformComponent->GetTransform().SetTranslation(cameraFront);
	}
}

void IndustryCameraController::MoveBackward(float amount)
{
	MoveForward(-amount);
}

void IndustryCameraController::MoveLeft(float amount)
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		cd::Matrix4x4 rotMatrix = pTransformComponent->GetTransform().GetRotation().ToMatrix4x4();
		cd::Vec3f transVec = pTransformComponent->GetTransform().GetTranslation();
		cd::Vec3f cameraCross(-rotMatrix.Data(0), rotMatrix.Data(1), rotMatrix.Data(2));
		cameraCross = cameraCross * amount + transVec;
		pTransformComponent->GetTransform().SetTranslation(cameraCross);
	}
}

void IndustryCameraController::MoveRight(float amount)
{
	MoveLeft(-amount);
}

void IndustryCameraController::Rotate(const cd::Vec3f& axis, float angleDegrees)
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		cd::Quaternion rotation = cd::Quaternion::FromAxisAngle(axis, cd::Math::DegreeToRadian<float>(angleDegrees));
		pTransformComponent->GetTransform().SetRotation(rotation * pTransformComponent->GetTransform().GetRotation());
	}
}

void IndustryCameraController::Rotate(float x, float y, float z, float angleDegrees)
{
	Rotate(cd::Vec3f(x, y, z), angleDegrees);
}

void IndustryCameraController::Yaw(float angleDegrees)
{
	//if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	//{
	//	cd::Matrix4x4 rotMatrix = pTransformComponent->GetTransform().GetRotation().ToMatrix4x4();
	//	cd::Vec3f cameraCross(-rotMatrix.Data(0), rotMatrix.Data(1), rotMatrix.Data(2));
	//	Rotate(cameraCross, angleDegrees);
	//}
}

void IndustryCameraController::Pitch(float angleDegrees)
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		cd::Matrix4x4 rotMatrix = pTransformComponent->GetTransform().GetRotation().ToMatrix4x4();
		cd::Vec3f cameraUp(rotMatrix.Data(4), rotMatrix.Data(5), rotMatrix.Data(6));
		Rotate(cameraUp, angleDegrees);
	}
}
}