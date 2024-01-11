#include "StaticMeshComponent.h"

#include "ECWorld/World.h"
#include "HalfEdgeMesh/HalfEdgeMesh.h"
#include "Log/Log.h"
#include "ProgressiveMesh/ProgressiveMesh.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/VertexFormat.h"
#include "Utilities/MeshUtils.hpp"

#include <bgfx/bgfx.h>

#include <optional>

namespace
{

uint16_t SubmitVertexBuffer(const std::vector<std::byte>& vertexBuffer, const cd::VertexFormat* pVertexFormat)
{
	bgfx::VertexLayout vertexLayout;
	engine::VertexLayoutUtility::CreateVertexLayout(vertexLayout, pVertexFormat->GetVertexLayout());
	const bgfx::Memory* pVertexBufferRef = bgfx::makeRef(vertexBuffer.data(), static_cast<uint32_t>(vertexBuffer.size()));
	bgfx::VertexBufferHandle vertexBufferHandle = bgfx::createVertexBuffer(pVertexBufferRef, vertexLayout);
	assert(bgfx::isValid(vertexBufferHandle));
	return vertexBufferHandle.idx;
}

enum class IndexBufferType
{
	Static,
	Dynamic
};

template<IndexBufferType IBT = IndexBufferType::Static>
uint16_t SubmitIndexBuffer(const std::vector<std::byte>& indexBuffer, bool useU16Index = false)
{
	const bgfx::Memory* pIndexBufferRef = bgfx::makeRef(indexBuffer.data(), static_cast<uint32_t>(indexBuffer.size()));
	if constexpr (IndexBufferType::Static == IBT)
	{
		bgfx::IndexBufferHandle indexBufferHandle = bgfx::createIndexBuffer(pIndexBufferRef, useU16Index ? 0U : BGFX_BUFFER_INDEX32);
		assert(bgfx::isValid(indexBufferHandle));
		return indexBufferHandle.idx;
	}
	else if constexpr (IndexBufferType::Dynamic == IBT)
	{
		bgfx::DynamicIndexBufferHandle indexBufferHandle = bgfx::createDynamicIndexBuffer(pIndexBufferRef, useU16Index ? 0U : BGFX_BUFFER_INDEX32);
		assert(bgfx::isValid(indexBufferHandle));
		return indexBufferHandle.idx;
	}
	else
	{
		static_assert("Unsupported IndexBufferType.");
	}
};

}

namespace engine
{

uint32_t StaticMeshComponent::GetStartVertex() const
{
	return 0U;
}

uint32_t StaticMeshComponent::GetVertexCount() const
{
	return m_currentVertexCount;
}

uint16_t StaticMeshComponent::GetVertexBuffer() const
{
#ifdef EDITOR_MODE
	return IsProgressiveMeshValid() ? m_progressiveMeshVertexBufferHandle : m_vertexBufferHandle;
#else
	return m_vertexBufferHandle;
#endif
}

uint32_t StaticMeshComponent::GetStartIndex() const
{
	return 0U;
}

uint32_t StaticMeshComponent::GetPolygonCount() const
{
	return m_currentPolygonCount;
}

uint32_t StaticMeshComponent::GetIndexCount() const
{
#ifdef EDITOR_MODE
	return IsProgressiveMeshValid() ? GetPolygonCount() * 3U : UINT32_MAX;
#else
	return UINT32_MAX;
#endif
}

uint16_t StaticMeshComponent::GetIndexBuffer() const
{
#ifdef EDITOR_MODE
	return IsProgressiveMeshValid() ? m_progressiveMeshIndexBufferHandle : m_indexBufferHandle;
#else
	return m_indexBufferHandle;
#endif
}

void StaticMeshComponent::Reset()
{
	m_pMeshData = nullptr;
	m_pRequiredVertexFormat = nullptr;

	m_currentVertexCount = UINT32_MAX;
	m_currentPolygonCount = UINT32_MAX;

	m_vertexBuffer.clear();
	m_vertexBufferHandle = UINT16_MAX;

	m_indexBuffer.clear();
	m_indexBufferHandle = UINT16_MAX;

#ifdef EDITOR_MODE
	m_wireframeIndexBuffer.clear();
	m_wireframeIndexBufferHandle = UINT16_MAX;

	m_originVertexCount = UINT32_MAX;
	m_originPolygonCount = UINT32_MAX;
	m_progressiveMeshReductionPercent = 1.0f;

	m_progressiveMeshVertexBuffer.clear();
	m_progressiveMeshVertexBufferHandle = UINT16_MAX;

	m_progressiveMeshIndexBuffer.clear();
	m_progressiveMeshIndexBufferHandle = UINT16_MAX;
#endif
}

void StaticMeshComponent::Build()
{
	CD_ASSERT(m_pMeshData && m_pRequiredVertexFormat, "Input data is not ready.");

	if (!m_pMeshData->GetVertexFormat().IsCompatiableTo(*m_pRequiredVertexFormat))
	{
		CD_ERROR("Current mesh data is not compatiable to required vertex format.");
		return;
	}

	m_currentVertexCount = m_pMeshData->GetVertexCount();
	m_currentPolygonCount = m_pMeshData->GetPolygonCount();


	std::optional<cd::VertexBuffer> optVertexBuffer = cd::BuildVertexBufferForStaticMesh(*m_pMeshData, *m_pRequiredVertexFormat);
	if (!optVertexBuffer.has_value())
	{
		CD_ERROR("Failed to build mesh vertex buffer.");
		return;
	}
	m_vertexBuffer = cd::MoveTemp(optVertexBuffer.value());

	// Fill index buffer data.
	std::optional<cd::IndexBuffer> optIndexBuffer = cd::BuildIndexBufferesForPolygonGroup(*m_pMeshData, 0U);
	if (!optIndexBuffer.has_value())
	{
		CD_ERROR("Failed to build mesh index buffer.");
		return;
	}
	m_indexBuffer = cd::MoveTemp(optIndexBuffer.value());

#ifdef EDITOR_MODE
	BuildWireframeData();
#endif
}

void StaticMeshComponent::Submit()
{
	// Create vertex buffer.
	m_vertexBufferHandle = SubmitVertexBuffer(m_vertexBuffer, m_pRequiredVertexFormat);

	// Create index buffer.
	const bool useU16Index = m_currentVertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	m_indexBufferHandle = SubmitIndexBuffer(m_indexBuffer, useU16Index);

#ifdef EDITOR_MODE
	m_wireframeIndexBufferHandle = SubmitIndexBuffer(m_wireframeIndexBuffer, useU16Index);
#endif
}

#ifdef EDITOR_MODE

void StaticMeshComponent::BuildWireframeData()
{
	const uint32_t indicesCount = m_currentPolygonCount * 3U;
	const bool useU16Index = m_currentVertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);

	uint32_t wireframeIndicesCount = bgfx::topologyConvert(bgfx::TopologyConvert::TriListToLineList, nullptr, 0U,
		m_indexBuffer.data(), indicesCount, !useU16Index);
	m_wireframeIndexBuffer.resize(wireframeIndicesCount * indexTypeSize);
	bgfx::topologyConvert(bgfx::TopologyConvert::TriListToLineList, m_wireframeIndexBuffer.data(), static_cast<uint32_t>(m_wireframeIndexBuffer.size()),
		m_indexBuffer.data(), indicesCount, !useU16Index);
}

void StaticMeshComponent::BuildProgressiveMeshData()
{
	if (IsProgressiveMeshValid())
	{
		return;
	}

	assert(m_pMeshData && m_pRequiredVertexFormat);

	auto progressiveMesh = cd::ProgressiveMesh::FromIndexedMesh(*m_pMeshData);
	auto hem = cd::HalfEdgeMesh::FromIndexedMesh(*m_pMeshData);
	progressiveMesh.InitBoundary(cd::Mesh::FromHalfEdgeMesh(hem, cd::ConvertStrategy::BoundaryOnly));
	auto permutationMapPair = progressiveMesh.BuildCollapseOperations();
	m_permutation = cd::MoveTemp(permutationMapPair.first);
	m_map = cd::MoveTemp(permutationMapPair.second);

	m_originVertexCount = m_currentVertexCount;
	m_progressiveMeshTargetVertexCount = m_originVertexCount;
	m_originPolygonCount = m_currentPolygonCount;

	// Copy and modify buffer.
	assert(!m_vertexBuffer.empty());
	uint32_t vertexStride = m_pRequiredVertexFormat->GetStride();
	assert(vertexStride * m_currentVertexCount == m_vertexBuffer.size());

	// Create a vertex buffer sorted by collape order.
	m_progressiveMeshVertexBuffer.resize(m_vertexBuffer.size());
	for (uint32_t vertexIndex = 0U; vertexIndex < m_currentVertexCount; ++vertexIndex)
	{
		uint32_t newVertexIndex = m_permutation[vertexIndex];
		assert(newVertexIndex < m_currentVertexCount);
		std::memcpy(m_progressiveMeshVertexBuffer.data() + newVertexIndex * vertexStride, m_vertexBuffer.data() + vertexIndex * vertexStride, vertexStride);
	}

	// After sorting vertex buffer, modify index buffer accordingly.
	auto BuildIndexBuffer = [&]<typename IndexType>()
	{
		m_progressiveMeshIndexBuffer.resize(m_indexBuffer.size());
		for (uint32_t indexIndex = 0U; indexIndex < m_currentPolygonCount * 3U; ++indexIndex)
		{
			auto* pIndexData = reinterpret_cast<IndexType*>(m_indexBuffer.data() + indexIndex * sizeof(IndexType));
			IndexType index = *pIndexData;
			assert(m_permutation[index] < m_currentVertexCount);
			IndexType newIndex = static_cast<IndexType>(m_permutation[index]);
			std::memcpy(m_progressiveMeshIndexBuffer.data() + indexIndex * sizeof(IndexType), &newIndex, sizeof(IndexType));
		}
	};

	const bool useU16Index = m_originVertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	if (useU16Index)
	{
		// The usage is strange for C++20 template lambda if it doesn't accept an expected type parameter to deduce automatically.
		// See https://stackoverflow.com/questions/66182453/c20-template-lambda-how-to-specify-template-argument.
		BuildIndexBuffer.template operator()<uint16_t>();
	}
	else
	{
		BuildIndexBuffer.template operator()<uint32_t>();
	}

	// Submit
	m_progressiveMeshVertexBufferHandle = SubmitVertexBuffer(m_progressiveMeshVertexBuffer, m_pRequiredVertexFormat);
	m_progressiveMeshIndexBufferHandle = SubmitIndexBuffer<IndexBufferType::Dynamic>(m_progressiveMeshIndexBuffer, useU16Index);
}

void StaticMeshComponent::UpdateProgressiveMeshData()
{
	assert(m_progressiveMeshReductionPercent >= 0.0f && m_progressiveMeshReductionPercent <= 1.0f);
	uint32_t percentVertexCount = static_cast<uint32_t>(m_progressiveMeshReductionPercent * m_originVertexCount);
	uint32_t finalVertexCount = std::min(m_progressiveMeshTargetVertexCount, percentVertexCount);
	if (finalVertexCount != m_currentVertexCount)
	{
		UpdateProgressiveMeshData(finalVertexCount);
	}
}

void StaticMeshComponent::UpdateProgressiveMeshData(uint32_t vertexCount)
{
	// update vertex used count
	m_currentVertexCount = vertexCount;

	// update index buffer
	auto UpdateIndexBuffer = [&]<typename IndexType>() -> uint32_t
	{
		uint32_t validPolygonCount = 0U;
		auto* pIndexBuffer = reinterpret_cast<IndexType*>(m_progressiveMeshIndexBuffer.data());
		for (uint32_t polygonIndex = 0U; polygonIndex < m_originPolygonCount; ++polygonIndex)
		{
			IndexType polygon[3];

			uint32_t startIndex = polygonIndex * 3;
			for (uint32_t ii = 0U; ii < 3U; ++ii)
			{
				IndexType index = pIndexBuffer[startIndex + ii];
				while (index > m_currentVertexCount)
				{
					index = m_map[index];
				}

				polygon[ii] = index;
			}

			const bool isPolygonValid = polygon[0] != polygon[1] && polygon[0] != polygon[2] && polygon[1] != polygon[2];
			if (isPolygonValid)
			{
				std::memcpy(pIndexBuffer + startIndex, polygon, 3 * sizeof(IndexType));
				++validPolygonCount;
			}
		}

		return validPolygonCount;
	};

	const bool useU16Index = m_originVertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	uint32_t polygonCount = useU16Index ? UpdateIndexBuffer.template operator() < uint16_t > () : UpdateIndexBuffer.template operator() < uint32_t > ();
	m_currentPolygonCount = polygonCount;
	bgfx::update(bgfx::DynamicIndexBufferHandle{ m_progressiveMeshIndexBufferHandle }, 0U,
		bgfx::makeRef(m_progressiveMeshIndexBuffer.data(), static_cast<uint32_t>(m_progressiveMeshIndexBuffer.size())));
}

#endif

}