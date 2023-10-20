#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/Entity.h"
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

	const cd::Mesh* GetMeshData() const { return m_pMeshData; }
	void SetMeshData(const cd::Mesh* pMeshData) { m_pMeshData = pMeshData; }
	void SetRequiredVertexFormat(const cd::VertexFormat* pVertexFormat) { m_pRequiredVertexFormat = pVertexFormat; }

	uint32_t GetStartVertex() const;
	uint32_t GetVertexCount() const;
	uint16_t GetVertexBuffer() const;
	uint32_t GetStartIndex() const;
	uint32_t GetPolygonCount() const;
	uint32_t GetIndexCount() const;
	uint16_t GetIndexBuffer() const;

	void Reset();
	void Build();
	void Submit();

private:
	// Input
	const cd::Mesh* m_pMeshData = nullptr;
	const cd::VertexFormat* m_pRequiredVertexFormat = nullptr;

	// Output
	uint32_t m_currentVertexCount = UINT32_MAX;
	uint32_t m_currentPolygonCount = UINT32_MAX;
	std::vector<std::byte> m_vertexBuffer;
	std::vector<std::byte> m_indexBuffer;
	uint16_t m_vertexBufferHandle = UINT16_MAX;
	uint16_t m_indexBufferHandle = UINT16_MAX;

#ifdef EDITOR_MODE
public:
	uint16_t GetWireframeIndexBuffer() const { return m_wireframeIndexBufferHandle; }

	bool IsProgressiveMeshValid() const { return m_progressiveMeshIndexBufferHandle != UINT16_MAX; }
	uint16_t GetProgressiveMeshIndexBuffer() const { return m_progressiveMeshIndexBufferHandle; }
	void BuildProgressiveMeshData();

	float GetProgressiveMeshReductionPercent() const { return m_progressiveMeshReductionPercent; }
	float& GetProgressiveMeshReductionPercent() { return m_progressiveMeshReductionPercent; }

	uint32_t GetOriginVertexCount() { return m_originVertexCount; }
	uint32_t& GetProgressiveMeshTargetVertexCount() { return m_progressiveMeshTargetVertexCount; }

	void UpdateProgressiveMeshData();
	void UpdateProgressiveMeshData(uint32_t vertexCount);

private:
	void BuildWireframeData();

private:
	std::vector<std::byte> m_wireframeIndexBuffer;
	uint16_t m_wireframeIndexBufferHandle = UINT16_MAX;

	std::vector<std::byte> m_progressiveMeshVertexBuffer;
	std::vector<std::byte> m_progressiveMeshIndexBuffer;
	uint16_t m_progressiveMeshIndexBufferHandle = UINT16_MAX;
	uint16_t m_progressiveMeshVertexBufferHandle = UINT16_MAX;

	uint32_t m_originVertexCount = UINT32_MAX;
	uint32_t m_originPolygonCount = UINT32_MAX;

	uint32_t m_progressiveMeshTargetVertexCount = UINT32_MAX;
	float m_progressiveMeshReductionPercent = 1.0f;
	std::vector<uint32_t> m_permutation;
	std::vector<uint32_t> m_map;
#endif
};

}