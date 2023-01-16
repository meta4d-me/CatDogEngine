#include "Camera.h"

namespace engine
{

Camera::Camera()
	: m_position(0.0f, 0.0f, 0.0f)
	, m_forwardDirection(0.0f, 0.0f, 1.0f)
	, m_upDirection(0.0f, 1.0f, 0.0f)
{
}

Camera::Camera(cd::Vec3f position)
	: m_position(std::move(position))
	, m_forwardDirection(0.0f, 0.0f, 1.0f)
	, m_upDirection(0.0f, 1.0f, 0.0f)
{
}

Camera::Camera(cd::Vec3f position, cd::Vec3f forward, cd::Vec3f up)
	: m_position(std::move(position))
	, m_forwardDirection(forward.Normalize())
	, m_upDirection(up.Normalize())
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
	cd::Point lookAt = aabb.Center();
	cd::Direction lookDirection(-1.0f, 0.0f, 1.0f);
	lookDirection.Normalize();

	cd::Point lookFrom = lookAt - lookDirection * aabb.Size().Length();

	m_position.x() = lookFrom.x();
	m_position.y() = lookFrom.y();
	m_position.z() = lookFrom.z();

	m_forwardDirection.x() = lookDirection.x();
	m_forwardDirection.y() = lookDirection.y();
	m_forwardDirection.z() = lookDirection.z();

	Dirty();
}

cd::Ray Camera::EmitRay(float screenX, float screenY, float width, float height)
{
	// TODO : Optimize inverse ProjectionMatrix * ViewMatrix twice?
	cd::Point rayStartPosition = cd::Matrix4x4::UnProject(cd::Vec3f(screenX, screenY, 0.0f),
		cd::Matrix4x4::Identity(), GetProjectionMatrix(), cd::Vec4f(0.0f, 0.0f, width, height));
	cd::Point rayEndPosition = cd::Matrix4x4::UnProject(cd::Vec3f(screenX, screenY, 1.0f),
		cd::Matrix4x4::Identity(), GetProjectionMatrix(), cd::Vec4f(0.0f, 0.0f, width, height));

	return cd::Ray(rayStartPosition, rayEndPosition - rayStartPosition);
}

const cd::Matrix4x4& Camera::GetViewMatrix()
{
	if (m_dirty)
	{
		UpdateViewMatrix();
	}

	return m_viewMatrix;
}

const cd::Matrix4x4& Camera::GetProjectionMatrix()
{
	if (m_dirty)
	{
		UpdateProjectionMatrix();
	}

	return m_projectionMatrix;
}

void Camera::UpdateViewMatrix()
{
	m_viewMatrix = cd::Matrix4x4::LookAt<cd::Handedness::Left>(m_position, m_position + m_forwardDirection, m_upDirection);
}

void Camera::UpdateProjectionMatrix()
{
	m_projectionMatrix = cd::Matrix4x4::Perspective(m_fov, m_aspect, m_nearPlane, m_farPlane, m_homogeneousNdc);
}

}