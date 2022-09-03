// Engine
#include "Camera.h"

// C/C++
#include <utility>

namespace engine
{

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
	m_near = nearPlane;
}

void Camera::SetFarPlane(float farPlane)
{
	Dirty();
	m_far = farPlane;
}

void Camera::SetHomogeneousNdc(bool homogeneousNdc)
{
	Dirty();
	m_homogeneousNdc = homogeneousNdc;
}

void Camera::SetEyePosition(bx::Vec3 eye)
{
	Dirty();
	m_eye = std::move(eye);
}

void Camera::SetLookTargetPosition(bx::Vec3 lookAt)
{
	Dirty();
	m_lookTarget = std::move(lookAt);
}

void Camera::SetUpDirection(bx::Vec3 up)
{
	Dirty();
	m_up = std::move(up);
}

void Camera::Update()
{
	if (!m_dirty) return;

	m_dirty = false;

	bx::mtxLookAt(m_viewMatrix, m_eye, m_lookTarget, m_up);
	bx::mtxProj(m_projectionMatrix, m_fov, m_aspect, m_near, m_far, m_homogeneousNdc);
}

const float* Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}

const float* Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}

}