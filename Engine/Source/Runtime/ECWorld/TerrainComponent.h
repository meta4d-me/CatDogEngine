#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/Entity.h"
#include "Math/Box.hpp"
#include "Scene/Mesh.h"

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

class TerrainComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("StaticMeshComponent");
		return className;
	}

public:
	TerrainComponent() = default;
	TerrainComponent(const TerrainComponent&) = default;
	TerrainComponent& operator=(const TerrainComponent&) = default;
	TerrainComponent(TerrainComponent&&) = default;
	TerrainComponent& operator=(TerrainComponent&&) = default;
	~TerrainComponent() = default;

	const cd::Mesh* GetMeshData() const { return m_pMeshData; }
	void SetMeshData(const cd::Mesh* pMeshData) { m_pMeshData = pMeshData; }
	void SetRequiredVertexFormat(const cd::VertexFormat* pVertexFormat) { m_pRequiredVertexFormat = pVertexFormat; }

	const cd::AABB& GetAABB() const { return m_aabb; }
	uint16_t GetVertexBuffer() const { return m_vertexBufferHandle; }
	uint16_t GetIndexBuffer() const { return m_indexBufferHandle; }
	uint16_t GetAABBVertexBuffer() const { return m_aabbVBH; }
	uint16_t GetAABBIndexBuffer() const { return m_aabbIBH; }

	void Reset();
	void Build();

private:
	void BuildDebug();

private:
	// Input
	uint32_t width;
	uint32_t depth;
	uint32_t patchSize;
	const cd::Mesh* m_pMeshData = nullptr;
	const cd::VertexFormat* m_pRequiredVertexFormat = nullptr;

	// Output
	//std::vector<LodInfo>
	std::vector<std::byte> m_vertexBuffer;
	std::vector<std::byte> m_indexBuffer;
	uint16_t m_vertexBufferHandle = UINT16_MAX;
	uint16_t m_indexBufferHandle = UINT16_MAX;

	// For debug use
	cd::AABB m_aabb;
	std::vector<std::byte> m_aabbVertexBuffer;
	std::vector<std::byte> m_aabbIndexBuffer;
	uint16_t m_aabbVBH = UINT16_MAX;
	uint16_t m_aabbIBH = UINT16_MAX;
};

}