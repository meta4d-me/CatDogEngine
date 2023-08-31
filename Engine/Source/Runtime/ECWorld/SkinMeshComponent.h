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


	uint16_t GetBonePositionBuffer() const { return m_boneVBH; }
	uint16_t GetIndexBuffer() const { return m_boneIBH; }

	void Reset();
	void Build();

private:


	//output
	std::vector<std::byte> m_vertexBuffer;
	std::vector<std::byte> m_indexBuffer;
	uint16_t m_boneVBH = UINT16_MAX;
	uint16_t m_boneIBH = UINT16_MAX;
};
}