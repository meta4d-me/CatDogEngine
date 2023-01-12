#include "TransformComponent.h"

namespace engine
{

void TransformComponent::Reset()
{
	m_transform.Clear();
	m_localToWorldMatrix.Clear();
}

void TransformComponent::Build()
{
	m_localToWorldMatrix = cd::Matrix4x4::Transform(m_transform.GetScale(), m_transform.GetRotation().ToMatrix3x3(), m_transform.GetScale());
}

}