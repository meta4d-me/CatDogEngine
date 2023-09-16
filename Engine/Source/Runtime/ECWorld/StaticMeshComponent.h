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

	uint16_t GetVertexBuffer() const { return m_vertexBufferHandle; }
	uint16_t GetIndexBuffer() const { return m_indexBufferHandle; }
#ifdef EDITOR_MODE
	uint16_t GetWireframeIndexBuffer() const { return m_wireframeIndexBufferHandle; }
#endif

	void Reset();
	void Build();
	void Submit();

private:
	// Input
	const cd::Mesh* m_pMeshData = nullptr;
	const cd::VertexFormat* m_pRequiredVertexFormat = nullptr;

	// Output
	std::vector<std::byte> m_vertexBuffer;
	std::vector<std::byte> m_indexBuffer;
	uint16_t m_vertexBufferHandle = UINT16_MAX;
	uint16_t m_indexBufferHandle = UINT16_MAX;

#ifdef EDITOR_MODE
	std::vector<std::byte> m_wireframeIndexBuffer;
	uint16_t m_wireframeIndexBufferHandle = UINT16_MAX;
#endif
};

}