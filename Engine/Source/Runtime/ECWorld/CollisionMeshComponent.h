#pragma once

#include "Core/StringCrc.h"

namespace engine
{

class CollisionMeshComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("CollisionMeshComponent");
		return className;
	}

public:
	CollisionMeshComponent() = default;
	CollisionMeshComponent(const CollisionMeshComponent&) = default;
	CollisionMeshComponent& operator=(const CollisionMeshComponent&) = default;
	CollisionMeshComponent(CollisionMeshComponent&&) = default;
	CollisionMeshComponent& operator=(CollisionMeshComponent&&) = default;
	~CollisionMeshComponent() = default;
};

}