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

	void Reset();
	void Build();
};

}