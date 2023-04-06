#include "CameraComponent.h"

namespace engine
{

void CameraComponent::Build()
{
	if (m_isViewDirty)
	{
		m_lookAt.Normalize();
		m_up.Normalize();
		m_cross = m_up.Cross(m_lookAt);
		m_viewMatrix = cd::Matrix4x4::LookAt<cd::Handedness::Left>(m_eye, m_eye + m_lookAt, m_up);
		m_isViewDirty = false;
	}

	if (m_isProjectionDirty)
	{
		m_projectionMatrix = cd::Matrix4x4::Perspective(m_fov, m_aspect, m_nearPlane, m_farPlane, cd::NDCDepth::MinusOneToOne == m_ndcDepth);
		m_isProjectionDirty = false;
	}
}

void CameraComponent::FrameAll(const cd::AABB& aabb)
{
	if (aabb.IsEmpty())
	{
		return;
	}

	cd::Point lookAt = aabb.Center();
	cd::Direction lookDirection(0.0f, 0.0f, 1.0f);
	lookDirection.Normalize();

	cd::Point lookFrom = lookAt - lookDirection * aabb.Size().Length();
	m_eye = lookFrom;
	m_lookAt = lookDirection;
	m_isViewDirty = true;
}

cd::Ray CameraComponent::EmitRay(float screenX, float screenY, float width, float height) const
{
	cd::Matrix4x4 vpInverse = m_projectionMatrix * m_viewMatrix;
	vpInverse = vpInverse.Inverse();

	float x = cd::Math::GetValueInNewRange(screenX / width, 0.0f, 1.0f, -1.0f, 1.0f);
	float y = cd::Math::GetValueInNewRange(screenY / height, 0.0f, 1.0f, -1.0f, 1.0f);

	cd::Vec4f near = vpInverse * cd::Vec4f(x, -y, 0.0f, 1.0f);
	near /= near.w();

	cd::Vec4f far = vpInverse * cd::Vec4f(x, -y, 1.0f, 1.0f);
	far /= far.w();

	cd::Vec4f direction = (far - near).Normalize();
	return cd::Ray(cd::Vec3f(near.x(), near.y(), near.z()),
		cd::Vec3f(direction.x(), direction.y(), direction.z()));
}

}