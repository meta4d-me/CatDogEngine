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
	m_localToWorldMatrix = m_transform.GetMatrix();
}

}