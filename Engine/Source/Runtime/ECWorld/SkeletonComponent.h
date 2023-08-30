#pragma once
#include <Core/StringCrc.h>

namespace engine
{

class SkeletonComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("StaticMeshComponent");
		return className;
	}

public:
	SkeletonComponent() = default;
	SkeletonComponent(const SkeletonComponent&) = default;
	SkeletonComponent& operator=(const SkeletonComponent&) = default;
	SkeletonComponent(SkeletonComponent&&) = default;
	SkeletonComponent& operator=(SkeletonComponent&&) = default;
	~SkeletonComponent() = default;

private:
	//Input

};
}