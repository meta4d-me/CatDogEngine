#include "SkinMeshComponent.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/Mesh.h"
#include "Scene/VertexFormat.h"


namespace engine
{

void SkinMeshComponent::Reset()
{
	m_vertexBuffer.clear();
	m_boneVBH = UINT16_MAX;

	m_indexBuffer.clear();
	m_boneIBH = UINT16_MAX;
}

void SkinMeshComponent::Build()
{

}
}