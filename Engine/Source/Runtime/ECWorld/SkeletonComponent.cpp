#include "SkeletonComponent.h"
#include "Rendering/Resources/SkeletonResource.h"

namespace engine
{

namespace details
{

}

void SkeletonComponent::Reset()
{

}

void SkeletonComponent::Build()
{

}

void SkeletonComponent::SetSkeletonAsset(const SkeletonResource* pSkeletonResource)
{
	int boneCount = pSkeletonResource->GetBoneCount();
	m_pSkeletonResource = pSkeletonResource;
	m_boneIBH = m_pSkeletonResource->GetIndexBufferHandle();
	m_boneVBH = m_pSkeletonResource->GetVertexBufferHandle();
	m_boneGlobalMatrices.resize(boneCount, cd::Matrix4x4::Identity());
	m_boneMatrices.resize(boneCount, cd::Matrix4x4::Identity());
}

void SkeletonComponent::SetBoneGlobalMatrix(uint32_t index, const cd::Matrix4x4& boneGlobalMatrix)
{
	m_boneGlobalMatrices[index] = boneGlobalMatrix;
	m_boneIndex = index;
}

}