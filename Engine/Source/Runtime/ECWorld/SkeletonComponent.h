#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/Entity.h"
#include "Scene/Bone.h"
#include <Rendering/Resources/SkeletonResource.h>

namespace cd
{

class Skeleton;

}

namespace engine
{

class SkeletonComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("SkeletonComponent");
		return className;
	}

public:
	SkeletonComponent() = default;
	SkeletonComponent(const SkeletonComponent&) = default;
	SkeletonComponent& operator=(const SkeletonComponent&) = default;
	SkeletonComponent(SkeletonComponent&&) = default;
	SkeletonComponent& operator=(SkeletonComponent&&) = default;
	~SkeletonComponent() = default;

	const SkeletonResource* GetSkeletonResource() const { return m_pSkeletonResource; }
	void SetSkeletonAsset(const SkeletonResource* pSkeletonResource);

	void SetBoneMatricesUniform(uint16_t uniform) { m_boneMatricesUniform = uniform; }
	uint16_t GetBoneMatrixsUniform() const { return m_boneMatricesUniform; }

	void SetBoneVBH(uint16_t boneVBH) { m_boneVBH = boneVBH; }
	uint16_t GetBoneVBH() const { return m_boneVBH; }

	void SetBoneIBH(uint16_t boneIBH) { m_boneIBH = boneIBH; }
	uint16_t GetBoneIBH() const { return m_boneIBH; }

	void SetBoneGlobalMatrix(uint32_t index, const cd::Matrix4x4& boneChangeMatrix);
	const cd::Matrix4x4& GetBoneGlobalMatrix(uint32_t index) { return m_boneGlobalMatrices[index]; }
	const std::vector<cd::Matrix4x4>& GetBoneGlobalMatrices() const { return m_boneGlobalMatrices; }

	void SetBoneMatrix(uint32_t index, const cd::Matrix4x4& changeMatrix) { m_boneMatrices[index] = changeMatrix * m_boneMatrices[index]; }
	const cd::Matrix4x4& GetBoneMatrix(uint32_t index) { return m_boneMatrices[index]; }

	cd::Matrix4x4& GetRootMatrix() { return m_curRootMatrix; }
	void SetRootMatrix(const cd::Matrix4x4& rootMatrix) { m_curRootMatrix = rootMatrix; }

	const cd::AnimationID& GetAnimation(uint32_t index) { return m_animationID[index]; }
	void AddAnimationID(uint32_t animationID) { m_animationID.push_back(animationID); }

	void Reset();
	void Build();

private:
	//input
	uint32_t m_boneIndex = engine::INVALID_ENTITY;
	const SkeletonResource* m_pSkeletonResource = nullptr;
	//output
	uint16_t m_boneVBH = UINT16_MAX;
	uint16_t m_boneIBH = UINT16_MAX;
	uint16_t m_boneMatricesUniform;

	cd::Matrix4x4 m_curRootMatrix = cd::Matrix4x4::Identity();
	std::vector<cd::Matrix4x4> m_boneGlobalMatrices;
	std::vector<cd::Matrix4x4> m_boneMatrices;
	std::vector<cd::AnimationID> m_animationID;

};

}