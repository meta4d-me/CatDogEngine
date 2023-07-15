#include "CameraComponent.h"
#include "Display/CameraUtility.h"
namespace engine
{

void CameraComponent::BuildView(cd::Transform transform)
{
	if (1)
	{
		cd::Vec3f lookAt = GetLookAt(transform).Normalize();
		cd::Vec3f up = GetUp(transform).Normalize();
		cd::Vec3f eye = transform.GetTranslation();
		m_viewMatrix = cd::Matrix4x4::LookAt<cd::Handedness::Left>(eye, eye + lookAt, up);
		m_isViewDirty = false;
		if (m_isProjectionDirty)
		{
			m_projectionMatrix = cd::Matrix4x4::Perspective(m_fov, m_aspect, m_nearPlane, m_farPlane, cd::NDCDepth::MinusOneToOne == m_ndcDepth);
			m_isProjectionDirty = false;
		}
	}
}

void CameraComponent::BuildProject()
{
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
	m_isViewDirty = true;
}

cd::Ray CameraComponent::EmitRay(float screenX, float screenY, float width, float height) const
{
	cd::Matrix4x4 vpInverse = m_projectionMatrix * m_viewMatrix;
	vpInverse = vpInverse.Inverse();

	float x = cd::Math::GetValueInNewRange(screenX / width, 0.0f, 1.0f, -1.0f, 1.0f);
	float y = cd::Math::GetValueInNewRange(screenY / height, 0.0f, 1.0f, -1.0f, 1.0f);

	cd::Vec4f near1 = vpInverse * cd::Vec4f(x, -y, 0.0f, 1.0f);
	near1 /= near1.w();

	cd::Vec4f far1 = vpInverse * cd::Vec4f(x, -y, 1.0f, 1.0f);
	far1 /= far1.w();

	cd::Vec4f direction = (far1 - near1).Normalize();
	return cd::Ray(cd::Vec3f(near1.x(), near1.y(), near1.z()),
		cd::Vec3f(direction.x(), direction.y(), direction.z()));
}

}