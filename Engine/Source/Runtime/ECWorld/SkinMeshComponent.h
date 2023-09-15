#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/Entity.h"
#include "Scene/Bone.h"

namespace cd
{

class Bone;

}

namespace engine
{

class SkinMeshComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("SkinMeshComponent");
		return className;
	}

public:
	SkinMeshComponent() = default;
	SkinMeshComponent(const SkinMeshComponent&) = default;
	SkinMeshComponent& operator=(const SkinMeshComponent&) = default;
	SkinMeshComponent(SkinMeshComponent&&) = default;
	SkinMeshComponent& operator=(SkinMeshComponent&&) = default;
	~SkinMeshComponent() = default;

	void SetBoneMatricesUniform(uint16_t uniform) { m_boneMatricesUniform = uniform; }
	uint16_t GetBoneMatrixsUniform() const { return m_boneMatricesUniform; }

	void SetBoneVBH(uint16_t boneVBH) { m_boneVBH = boneVBH; }
	uint16_t GetBoneVBH() const { return m_boneVBH; }

	void SetBoneIBH(uint16_t boneIBH) { m_boneIBH = boneIBH; }
	uint16_t GetBoneIBH() const { return m_boneIBH; }

	void SetBoneMatricesSize(uint32_t boneCount);
	void SetBoneChangeMatrix(uint32_t index, const cd::Matrix4x4& boneChangeMatrix);
	const cd::Matrix4x4& GetBoneChangeMatrix(uint32_t index) { return m_changeMatrices[index]; }
	const std::vector<cd::Matrix4x4>& GetBoneChangeMatrices() const { return m_changeMatrices; }
	uint32_t GetChangeBoneIndex() const { return m_changeBoneIndex; }
	void SetChangeBoneIndex(uint32_t index) {m_changeBoneIndex = index;}
	void ResetChangeBoneIndex() { m_changeBoneIndex = engine::INVALID_ENTITY; }

	void SetBoneMatrix(uint32_t index, const cd::Matrix4x4& changeMatrix) { m_boneMatrices[index] = changeMatrix * m_boneMatrices[index]; }
	const cd::Matrix4x4& GetBoneMatrix(uint32_t index) { return m_boneMatrices[index]; }

	void Reset();
	void Build();

private:
	//input
	uint32_t m_changeBoneIndex = engine::INVALID_ENTITY;

	//output
	uint16_t m_boneVBH = UINT16_MAX;
	uint16_t m_boneIBH = UINT16_MAX;
	uint16_t m_boneMatricesUniform;

	std::vector<cd::Matrix4x4> m_changeMatrices;
	std::vector<cd::Matrix4x4> m_boneMatrices;
};

}