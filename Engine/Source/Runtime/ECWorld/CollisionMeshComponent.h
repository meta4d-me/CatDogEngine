#pragma once

#include "Core/StringCrc.h"
#include "Math/Box.hpp"

#include <vector>

namespace engine
{

enum class CollisonMeshType
{
	AABB,
	OBB
};

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

	void SetType(CollisonMeshType collisionType) { m_collisionType = collisionType; }
	CollisonMeshType GetType() const { return m_collisionType; }

	void SetAABB(cd::AABB aabb) { m_aabb = cd::MoveTemp(aabb); }
	const cd::AABB& GetAABB() const { return m_aabb; }
	
	uint16_t GetVertexBuffer() const { return m_aabbVBH; }
	uint16_t GetIndexBuffer() const { return m_aabbIBH; }

	void Reset();
	void Build();

private:
	CollisonMeshType m_collisionType = CollisonMeshType::AABB;
	cd::AABB m_aabb;

	std::vector<std::byte> m_aabbVertexBuffer;
	std::vector<std::byte> m_aabbIndexBuffer;
	uint16_t m_aabbVBH = UINT16_MAX;
	uint16_t m_aabbIBH = UINT16_MAX;
};

}