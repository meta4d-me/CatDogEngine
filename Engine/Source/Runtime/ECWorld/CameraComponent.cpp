#include "CameraComponent.h"

namespace engine
{

void CameraComponent::Build(TransformComponent* pTransformcomponent)
{
	if (m_isProjectionDirty)
	{
		m_projectionMatrix = cd::Matrix4x4::Perspective(m_fov, m_aspect, m_nearPlane, m_farPlane, cd::NDCDepth::MinusOneToOne == m_ndcDepth);
		m_isProjectionDirty = false;
	}
	// add if
	{
		cd::Vec3f Translation = pTransformcomponent->GetTransform().GetTranslation();
		cd::Matrix3x3 Rotation = pTransformcomponent->GetTransform().GetRotation().ToMatrix3x3();
		cd::Vec3f xAxis(Rotation.Data(0), Rotation.Data(3), Rotation.Data(6));
		cd::Vec3f yAxis(Rotation.Data(1), Rotation.Data(4), Rotation.Data(7));
		cd::Vec3f zAxis(Rotation.Data(2), Rotation.Data(5), Rotation.Data(8));

		cd::Matrix4x4 matrix(xAxis.x(), yAxis.x(), zAxis.x(), 0.0f,
							 xAxis.y(), yAxis.y(), zAxis.y(), 0.0f,
							 xAxis.z(), yAxis.z(), zAxis.z(), 0.0f,
						    -xAxis.Dot(Translation), -yAxis.Dot(Translation), -zAxis.Dot(Translation), 1.0f);
		m_viewMatrix = matrix;
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
	/*m_eye = lookFrom;
	m_lookAt = lookDirection;
	m_isViewDirty = true;*/
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