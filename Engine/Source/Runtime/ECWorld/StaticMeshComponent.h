#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/Entity.h"
#include "Math/AABB.hpp"

#include <cstdint>
#include <vector>

namespace cd
{

class Mesh;
class VertexFormat;

}

namespace engine
{

class World;

class StaticMeshComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("StaticMeshComponent");
		return className;
	}

public:
	StaticMeshComponent() = default;
	StaticMeshComponent(const StaticMeshComponent&) = default;
	StaticMeshComponent& operator=(const StaticMeshComponent&) = default;
	StaticMeshComponent(StaticMeshComponent&&) = default;
	StaticMeshComponent& operator=(StaticMeshComponent&&) = default;
	~StaticMeshComponent() = default;

	void SetMeshData(const cd::Mesh* pMeshData) { m_pMeshData = pMeshData; }
	void SetRequiredVertexFormat(const cd::VertexFormat* pVertexFormat) { m_pRequiredVertexFormat = pVertexFormat; }

	const cd::AABB& GetAABB() const;
	uint16_t GetVertexBuffer() const { return m_vertexBufferHandle; }
	uint16_t GetIndexBuffer() const { return m_indexBufferHandle; }

	void Reset();
	void Build();

private:
	// Input
	const cd::Mesh* m_pMeshData = nullptr;
	const cd::VertexFormat* m_pRequiredVertexFormat = nullptr;

	// Output
	std::vector<std::byte> m_vertexBuffer;
	std::vector<std::byte> m_indexBuffer;
	uint16_t m_vertexBufferHandle = UINT16_MAX;
	uint16_t m_indexBufferHandle = UINT16_MAX;
};

}