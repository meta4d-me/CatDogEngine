#include "TransformComponent.h"

namespace engine
{

void TransformComponent::Build()
{
	m_transformation = cd::Matrix4x4::Transform(m_translation, m_rotation, m_scale);
}

}