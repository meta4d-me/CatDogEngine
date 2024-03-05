#include "StaticMeshComponent.h"

#include "Rendering/Resources/MeshResource.h"

namespace engine
{

uint32_t StaticMeshComponent::GetStartVertex() const
{
	return 0U;
}

uint32_t StaticMeshComponent::GetVertexCount() const
{
	return m_currentVertexCount;
}

uint32_t StaticMeshComponent::GetStartIndex() const
{
	return 0U;
}

uint32_t StaticMeshComponent::GetPolygonCount() const
{
	return m_currentPolygonCount;
}

uint32_t StaticMeshComponent::GetIndexCount() const
{
	return UINT32_MAX;
}

void StaticMeshComponent::SetMeshResource(const MeshResource* pMeshResource)
{
	m_pMeshResource = pMeshResource;
	m_currentVertexCount = m_pMeshResource->GetVertexCount();
	m_currentPolygonCount = m_pMeshResource->GetPolygonCount();
}

}