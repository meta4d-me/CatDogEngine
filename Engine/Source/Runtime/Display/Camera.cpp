#include "Camera.h"

namespace engine
{

Camera::Camera()
	: m_position(0.0f, 0.0f, 0.0f)
	, m_forwardDirection(0.0f, 0.0f, 1.0f)
	, m_upDirection(0.0f, 1.0f, 0.0f)
{
}

Camera::Camera(bx::Vec3 position)
	: m_position(std::move(position))
	, m_forwardDirection(0.0f, 0.0f, 1.0f)
	, m_upDirection(0.0f, 1.0f, 0.0f)
{
}

Camera::Camera(bx::Vec3 position, bx::Vec3 forward, bx::Vec3 up)
	: m_position(std::move(position))
	, m_forwardDirection(bx::normalize(std::move(forward)))
	, m_upDirection(bx::normalize(std::move(up)))
{
}

void Camera::SetAspect(float aspect)
{
	Dirty();
	m_aspect = aspect;
}

void Camera::SetFov(float fov)
{
	Dirty();
	m_fov = fov;
}

void Camera::SetNearPlane(float nearPlane)
{
	Dirty();
	m_nearPlane = nearPlane;
}

void Camera::SetFarPlane(float farPlane)
{
	Dirty();
	m_farPlane = farPlane;
}

void Camera::SetHomogeneousNdc(bool homogeneousNdc)
{
	Dirty();
	m_homogeneousNdc = homogeneousNdc;
}

void Camera::FrameAll(const cd::AABB& aabb)
{
	cd::Point lookAt = aabb.GetCenter();
	cd::Direction lookDirection(-1.0f, 0.0f, 1.0f);
	// Should be calculated at compile time. To verify.
	lookDirection.Normalize();

	cd::Point lookFrom = lookAt - lookDirection * aabb.GetExtents().Length();

	m_position.x = lookFrom.x();
	m_position.y = lookFrom.y();
	m_position.z = lookFrom.z();

	m_forwardDirection.x = lookDirection.x();
	m_forwardDirection.y = lookDirection.y();
	m_forwardDirection.z = lookDirection.z();

	Dirty();
}

void Camera::UpdateViewMatrix()
{
	bx::mtxLookAt(m_viewMatrix, m_position, bx::add(m_position, m_forwardDirection), m_upDirection);
}

void Camera::UpdateProjectionMatrix()
{
	bx::mtxProj(m_projectionMatrix, m_fov, m_aspect, m_nearPlane, m_farPlane, m_homogeneousNdc);
}

void Camera::Update()
{
	if(!m_dirty)
	{
		return;
	}

	UpdateViewMatrix();
	UpdateProjectionMatrix();

	m_dirty = false;
}

}