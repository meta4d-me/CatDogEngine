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
		Rotate(m_movementSpeed * deltaTime);
	}
}
CameraComponent* CameraController::GetMainCameraComponent()const
{
	return m_pSceneWorld->GetCameraComponent(m_pSceneWorld->GetMainCameraEntity());
}

TransformComponent* CameraController::GetTransformComponent()const
{
	return m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity());
}

void CameraController::Rotate(float amount)
{
	

	CameraComponent* pCameraComponent = GetMainCameraComponent();
	m_cameraPosition = pCameraComponent->GetEye();

	m_orbitCenter = cd::Vec3f(0, 0, 1);
	cd::Vec3f radius = m_orbitCenter - m_cameraPosition;
	float distance = radius.Length();
	m_horizontalAngle += amount * m_movementSpeed;
	m_horizontalAngle = std::fmod(m_horizontalAngle, 360.0f);

	cd::Vec3f newPosition = m_orbitCenter + cd::Vec3f(std::cos(cd::Math::DegreeToRadian(m_verticalAngle)) * std::sin(cd::Math::DegreeToRadian(m_horizontalAngle)),
		std::sin(cd::Math::DegreeToRadian(m_verticalAngle)),
		std::cos(cd::Math::DegreeToRadian(m_verticalAngle)) * std::cos(cd::Math::DegreeToRadian(m_horizontalAngle))) * distance;
	pCameraComponent->SetEye(newPosition);
	pCameraComponent->SetLookAt((m_orbitCenter - newPosition).Normalize());

	cd::Vec3f newUp = radius.Normalize().Cross(cd::Vec3f(1, 0, 0));
	newUp.y() = pCameraComponent->GetUp().y();
	pCameraComponent->SetUp(newUp);

	if (TransformComponent* pTransformComponent = m_pSceneWorld->GetTransformComponent(m_pSceneWorld->GetMainCameraEntity()))
	{
		pTransformComponent->GetTransform().SetTranslation(newPosition);
		cd::Quaternion rotY = cd::Quaternion::FromAxisAngle(cd::Vec3f(0, 1, 0),cd::Math::DegreeToRadian(m_horizontalAngle));
		pTransformComponent->GetTransform().SetRotation(rotY);
		pTransformComponent->Dirty();
		pTransformComponent->Build();
	}

	
}

}