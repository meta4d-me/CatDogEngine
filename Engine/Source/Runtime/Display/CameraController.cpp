#include"CameraController.h"

namespace engine
{
CameraController::CameraController(
	const SceneWorld* pSceneWorld,
	const float sensitivity,
	const float movement_speed)
	: CameraController(pSceneWorld, sensitivity, sensitivity, movement_speed)
{
}

CameraController::CameraController(
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

void CameraController::Update(float deltaTime)
{
	if (Input::Get().IsKeyPressed(KeyCode::o))
	{
		RotateHorizon(m_horizontalSensitivity * deltaTime);
	}
	if (Input::Get().IsKeyPressed(KeyCode::p))
	{
		RotatedVertical(m_verticalSensitivity * deltaTime);
	}
	//if (Input::Get().IsMouseRBPressed())
	//{
	//	RotateHorizon(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
	//	RotatedVertical(m_verticalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	//	CD_INFO(-m_horizontalSensitivity * Input::Get().GetMousePositionOffsetY() * deltaTime);
	//	CD_INFO(m_verticalSensitivity * Input::Get().GetMousePositionOffsetX() * deltaTime);
	//}
}
CameraComponent* CameraController::GetMainCameraComponent()const
{
	return m_pSceneWorld->GetCameraComponent(m_pSceneWorld->GetMainCameraEntity());
}

TransformComponent* CameraController::GetTransformComponent()const
{
	return m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity());
}

void CameraController::RotateHorizon(float amount)
{
	

	//CameraComponent* pCameraComponent = GetMainCameraComponent();
	//m_cameraPosition = pCameraComponent->GetEye();

	//m_orbitCenter = cd::Vec3f(0, 0, 1);
	//cd::Vec3f radius = m_orbitCenter - m_cameraPosition;
	//float distance = radius.Length();
	//m_horizontalAngle += amount * m_movementSpeed;
	//m_horizontalAngle = std::fmod(m_horizontalAngle, 360.0f);

	//cd::Vec3f newPosition = m_orbitCenter + cd::Vec3f(std::cos(cd::Math::DegreeToRadian(m_verticalAngle)) * std::sin(cd::Math::DegreeToRadian(m_horizontalAngle)),
	//	std::sin(cd::Math::DegreeToRadian(m_verticalAngle)),
	//	std::cos(cd::Math::DegreeToRadian(m_verticalAngle)) * std::cos(cd::Math::DegreeToRadian(m_horizontalAngle))) * distance;
	//pCameraComponent->SetEye(newPosition);
	//pCameraComponent->SetLookAt((m_orbitCenter - newPosition).Normalize());

	//cd::Vec3f newUp = radius.Normalize().Cross(cd::Vec3f(1, 0, 0));
	//newUp.y() = pCameraComponent->GetUp().y();
	//pCameraComponent->SetUp(newUp);

	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		m_cameraPosition = pTransformComponent->GetTransform().GetTranslation();
		cd::Transform transform = pTransformComponent->GetTransform();
		m_orbitCenter = cd::Vec3f(0, 0, 1);
		cd::Vec3f pc = m_cameraPosition - m_orbitCenter;
		CameraComponent* pCameraComponent = GetMainCameraComponent();

		float thetaVertical = std::atan2f(pc.z(), pc.y());
		m_verticalAngle = amount * m_verticalSensitivity;
		float verticalY = std::cos(cd::Math::DegreeToRadian(m_verticalAngle)) * pc.y() + std::sin(cd::Math::DegreeToRadian(m_verticalAngle)) * pc.z();
		float verticalZ = -std::sin(cd::Math::DegreeToRadian(m_verticalAngle)) * pc.y() + std::cos(cd::Math::DegreeToRadian(m_verticalAngle)) * pc.z();
		cd::Vec3f vertivalPosition = cd::Vec3f(pc.x(), verticalY, verticalZ) + m_orbitCenter;
		pTransformComponent->GetTransform().SetTranslation(vertivalPosition);
		float verticalAngle = std::acos(pc.Dot(vertivalPosition - m_orbitCenter) / (pc.Length() * (vertivalPosition - m_orbitCenter).Length()));
		cd::Quaternion verticalRotation = cd::Quaternion::FromAxisAngle(cd::Vec3f(-1, 0, 0), verticalAngle);
		pTransformComponent->GetTransform().SetRotation(verticalRotation * transform.GetRotation());
		pCameraComponent->Build(pTransformComponent);

		
	}

	
}

void CameraController::RotatedVertical(float amount)
{
	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		m_cameraPosition = pTransformComponent->GetTransform().GetTranslation();
		cd::Transform transform = pTransformComponent->GetTransform();
		m_orbitCenter = cd::Vec3f(0, 0, 1);
		cd::Vec3f pc = m_cameraPosition - m_orbitCenter;
		CameraComponent* pCameraComponent = GetMainCameraComponent();

		float thetaHorizon = std::atan2f(pc.z(), pc.x());
		m_horizontalAngle = amount * m_horizontalSensitivity;
		float horizonX = std::cos(cd::Math::DegreeToRadian(m_horizontalAngle)) * pc.x() + std::sin(cd::Math::DegreeToRadian(m_horizontalAngle)) * pc.z();
		float horizonZ = -std::sin(cd::Math::DegreeToRadian(m_horizontalAngle)) * pc.x() + std::cos(cd::Math::DegreeToRadian(m_horizontalAngle)) * pc.z();
		cd::Vec3f horizonPosition = cd::Vec3f(horizonX, pc.y(), horizonZ) + m_orbitCenter;
		pTransformComponent->GetTransform().SetTranslation(horizonPosition);
		float horizonAngle = std::acos(pc.Dot(horizonPosition - m_orbitCenter) / (pc.Length() * (horizonPosition - m_orbitCenter).Length()));
		cd::Quaternion horizonRotation = cd::Quaternion::FromAxisAngle(cd::Vec3f(0, 1, 0), horizonAngle);
		pTransformComponent->GetTransform().SetRotation(horizonRotation * transform.GetRotation());
		pCameraComponent->Build(pTransformComponent);
	}
}

}