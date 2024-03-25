#pragma once

#include "IResource.h"
#include "Scene/VertexFormat.h"

#include <vector>

namespace cd
{

class Mesh;

}

namespace engine
{

class MeshResource : public IResource
{
public:
	using VertexBuffer = std::vector<std::byte>;
	using IndexBuffer = std::vector<std::byte>;

public:
	MeshResource();
	MeshResource(const MeshResource&) = default;
	MeshResource& operator=(const MeshResource&) = default;
	MeshResource(MeshResource&&) = default;
	MeshResource& operator=(MeshResource&&) = default;
	virtual ~MeshResource();

	virtual void Update() override;
	virtual void Reset() override;

	const cd::Mesh* GetMeshAsset() const { return m_pMeshAsset; }
	void SetMeshAsset(const cd::Mesh* pMeshAsset);
	
	void UpdateVertexFormat(const cd::VertexFormat& vertexFormat);
	
	uint32_t GetVertexCount() const { return m_vertexCount; }
	uint32_t GetPolygonCount() const { return m_polygonCount; }
	uint32_t GetPolygonGroupCount() const { return m_polygonGroupCount; }
	uint16_t GetVertexBufferHandle() const;
	uint32_t GetIndexBufferCount() const { return static_cast<uint32_t>(m_indexBufferHandles.size()); }
	uint16_t GetIndexBufferHandle(uint32_t index) const;

private:
	bool BuildVertexBuffer();
	bool BuildIndexBuffer();
	void SubmitVertexBuffer();
	void SubmitIndexBuffer();
	void ClearMeshData();
	void FreeMeshData();
	void DestroyVertexBufferHandle();
	void DestroyIndexBufferHandle();

private:
	// Asset
	const cd::Mesh* m_pMeshAsset = nullptr;
	uint32_t m_vertexCount = 0U;
	uint32_t m_polygonCount = 0U;
	uint32_t m_polygonGroupCount = 0U;

	// Runtime
	cd::VertexFormat m_currentVertexFormat;

	// CPU
	VertexBuffer m_vertexBuffer;
	std::vector<IndexBuffer> m_indexBuffers;
	uint32_t m_recycleCount = 0;

	// GPU
	uint16_t m_vertexBufferHandle = UINT16_MAX;
	std::vector<uint16_t> m_indexBufferHandles;
};

}