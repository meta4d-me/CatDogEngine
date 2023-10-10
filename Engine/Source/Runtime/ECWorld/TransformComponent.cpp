#include "TransformComponent.h"

namespace engine
{

void TransformComponent::Reset()
{
	m_transform.Clear();
	m_localToWorldMatrix.Clear();
	m_isMatrixDirty = true;
}

void TransformComponent::Build()
{
	if (m_isMatrixDirty)
	{
		m_localToWorldMatrix = m_transform.GetMatrix();
		m_isMatrixDirty = false;
	}
}
#ifdef EDITOR_MODE
bool TransformComponent::m_doUseUniformScale = true;
#endif

}