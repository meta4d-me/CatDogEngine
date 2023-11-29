#include "SkinMeshComponent.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/Mesh.h"
#include "Scene/VertexFormat.h"

namespace engine
{

void SkinMeshComponent::Reset()
{

}

void SkinMeshComponent::Build()
{

}

void SkinMeshComponent::SetBoneMatricesSize(uint32_t boneCount)
{
	m_boneGlobalMatrices.resize(boneCount);
	m_boneMatrices.resize(boneCount);
	for (uint32_t i = 0; i < boneCount; ++i)
	{
		m_boneGlobalMatrices[i] = cd::Matrix4x4::Identity();
		m_boneMatrices[i] = cd::Matrix4x4::Identity();
	}
}

void SkinMeshComponent::SetBoneGlobalMatrix(uint32_t index, const cd::Matrix4x4& boneChangeMatrix)
{
	m_boneGlobalMatrices[index] = boneChangeMatrix;
	m_changeBoneIndex = index;
}

}