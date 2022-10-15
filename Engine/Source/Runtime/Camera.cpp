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

void Camera::Update()
{
	if (!m_dirty) return;

	bx::mtxProj(m_projectionMatrix, m_fov, m_aspect, m_near, m_far, m_homogeneousNdc);

	m_dirty = false;
}

const float* Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}

}