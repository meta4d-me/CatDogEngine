#include "StaticMeshComponent.h"

#include "ECWorld/World.h"
#include "Log/Log.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/VertexFormat.h"

#include <bgfx/bgfx.h>

#include <optional>

namespace
{

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
#ifdef EDITOR_MODE
	return IsProgressiveMeshValid() ? m_currentVertexCount : UINT32_MAX;
#else
	return UINT32_MAX;
#endif
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

uint32_t StaticMeshComponent::GetIndexCount() const
{
#ifdef EDITOR_MODE
	return IsProgressiveMeshValid() ? m_currentPolygonCount * 3U : UINT32_MAX;
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

	m_vertexBuffer.clear();
	m_vertexBufferHandle = UINT16_MAX;

	m_indexBuffer.clear();
	m_indexBufferHandle = UINT16_MAX;

#ifdef EDITOR_MODE
	m_wireframeIndexBuffer.clear();
	m_wireframeIndexBufferHandle = UINT16_MAX;

	m_totalVertexCount = UINT32_MAX;
	m_currentVertexCount = UINT32_MAX;
	m_totalPolygonCount = UINT32_MAX;
	m_currentPolygonCount = UINT32_MAX;
	m_progressiveMeshLODPercent = 1.0f;

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

	const bool containsPosition = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Position);
	const bool containsNormal = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Normal);
	const bool containsTangent = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Tangent);
	const bool containsBiTangent = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Bitangent);
	const bool containsUV = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::UV);
	const bool containsColor = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Color);

	// TODO : Store animation here temporarily to test.
	const bool containsBoneIndex = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::BoneIndex);
	const bool containsBoneWeight = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::BoneWeight);

	const uint32_t vertexCount = m_pMeshData->GetVertexCount();
	const uint32_t vertexFormatStride = m_pRequiredVertexFormat->GetStride();

	m_vertexBuffer.resize(vertexCount * vertexFormatStride);

	uint32_t vbDataSize = 0U;
	auto vbDataPtr = m_vertexBuffer.data();
	auto FillVertexBuffer = [&vbDataPtr, &vbDataSize](const void* pData, uint32_t dataSize)
	{
		std::memcpy(&vbDataPtr[vbDataSize], pData, dataSize);
		vbDataSize += dataSize;
	};

	for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
	{
		if (containsPosition)
		{
			constexpr uint32_t dataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
			FillVertexBuffer(m_pMeshData->GetVertexPosition(vertexIndex).Begin(), dataSize);
		}

		if (containsNormal)
		{
			constexpr uint32_t dataSize = cd::Direction::Size * sizeof(cd::Direction::ValueType);
			FillVertexBuffer(m_pMeshData->GetVertexNormal(vertexIndex).Begin(), dataSize);
		}

		if (containsTangent)
		{
			constexpr uint32_t dataSize = cd::Direction::Size * sizeof(cd::Direction::ValueType);
			FillVertexBuffer(m_pMeshData->GetVertexTangent(vertexIndex).Begin(), dataSize);
		}
		
		if (containsBiTangent)
		{
			constexpr uint32_t dataSize = cd::Direction::Size * sizeof(cd::Direction::ValueType);
			FillVertexBuffer(m_pMeshData->GetVertexBiTangent(vertexIndex).Begin(), dataSize);
		}
		
		if (containsUV)
		{
			constexpr uint32_t dataSize = cd::UV::Size * sizeof(cd::UV::ValueType);
			FillVertexBuffer(m_pMeshData->GetVertexUV(0)[vertexIndex].Begin(), dataSize);
		}

		if (containsColor)
		{
			constexpr uint32_t dataSize = cd::Color::Size * sizeof(cd::Color::ValueType);
			FillVertexBuffer(m_pMeshData->GetVertexColor(0)[vertexIndex].Begin(), dataSize);
		}

		if (containsBoneIndex && containsBoneWeight)
		{
			std::vector<uint16_t> vertexBoneIDs;
			std::vector<cd::VertexWeight> vertexBoneWeights;

			for(uint32_t vertexBoneIndex = 0U; vertexBoneIndex < 4; ++vertexBoneIndex)
			{
				cd::BoneID boneID;
				if (vertexBoneIndex < m_pMeshData->GetVertexInfluenceCount())
				{
					boneID = m_pMeshData->GetVertexBoneID(vertexBoneIndex, vertexIndex);
				}

				if (boneID.IsValid())
				{
					vertexBoneIDs.push_back(static_cast<uint16_t>(boneID.Data()));
					vertexBoneWeights.push_back(m_pMeshData->GetVertexWeight(vertexBoneIndex, vertexIndex));
				}
				else
				{
					vertexBoneIDs.push_back(127);
					vertexBoneWeights.push_back(0.0f);
				}
			}

			// TODO : Change storage to a TVector<uint16_t, InfluenceCount> and TVector<float, InfluenceCount> ?
			FillVertexBuffer(vertexBoneIDs.data(), static_cast<uint32_t>(vertexBoneIDs.size() * sizeof(uint16_t)));
			FillVertexBuffer(vertexBoneWeights.data(), static_cast<uint32_t>(vertexBoneWeights.size() * sizeof(cd::VertexWeight)));
		}
	}

	// Fill index buffer data.
	const bool useU16Index = vertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
	const uint32_t indicesCount = m_pMeshData->GetPolygonCount() * 3U;
	m_indexBuffer.resize(indicesCount* indexTypeSize);

	uint32_t ibDataSize = 0U;
	auto ibDataPtr = m_indexBuffer.data();
	auto FillIndexBuffer = [&ibDataPtr, &ibDataSize](const void* pData, uint32_t dataSize)
	{
		std::memcpy(&ibDataPtr[ibDataSize], pData, dataSize);
		ibDataSize += dataSize;
	};

	for (const auto& polygon : m_pMeshData->GetPolygons())
	{
		if (useU16Index)
		{
			// cd::Mesh always uses uint32_t to store index so it is not convenient to copy servals elements at the same time.
			for (auto vertexID : polygon)
			{
				uint16_t vertexIndex = static_cast<uint16_t>(vertexID.Data());
				FillIndexBuffer(&vertexIndex, indexTypeSize);
			}
		}
		else
		{
			FillIndexBuffer(polygon.data(), static_cast<uint32_t>(polygon.size() * indexTypeSize));
		}
	}

#ifdef EDITOR_MODE
	BuildWireframeData();
#endif
}

void StaticMeshComponent::Submit()
{
	// Create vertex buffer.
	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, m_pRequiredVertexFormat->GetVertexLayout());
	const bgfx::Memory* pVertexBufferRef = bgfx::makeRef(m_vertexBuffer.data(), static_cast<uint32_t>(m_vertexBuffer.size()));
	bgfx::VertexBufferHandle vertexBufferHandle = bgfx::createVertexBuffer(pVertexBufferRef, vertexLayout);
	assert(bgfx::isValid(vertexBufferHandle));
	m_vertexBufferHandle = vertexBufferHandle.idx;

	// Create index buffer.
	const bool useU16Index = m_pMeshData->GetVertexCount() <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	m_indexBufferHandle = SubmitIndexBuffer(m_indexBuffer, useU16Index);

#ifdef EDITOR_MODE
	m_wireframeIndexBufferHandle = SubmitIndexBuffer(m_wireframeIndexBuffer, useU16Index);
#endif
}

#ifdef EDITOR_MODE

void StaticMeshComponent::BuildWireframeData()
{
	assert(m_pMeshData);

	const uint32_t indicesCount = m_pMeshData->GetPolygonCount() * 3U;
	const bool useU16Index = m_pMeshData->GetVertexCount() <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);

	uint32_t wireframeIndicesCount = bgfx::topologyConvert(bgfx::TopologyConvert::TriListToLineList, nullptr, 0U,
		m_indexBuffer.data(), indicesCount, !useU16Index);
	m_wireframeIndexBuffer.resize(indicesCount * indexTypeSize);
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

	auto permutationMapPair = m_pMeshData->BuildProgressiveMesh();
	m_permutation = cd::MoveTemp(permutationMapPair.first);
	m_map = cd::MoveTemp(permutationMapPair.second);

	m_totalVertexCount = m_pMeshData->GetVertexCount();
	m_currentVertexCount = m_totalVertexCount;
	m_totalPolygonCount = m_pMeshData->GetPolygonCount();
	m_currentPolygonCount = m_totalPolygonCount;
	m_progressiveMeshIndexBuffer.resize(m_currentPolygonCount * 3U);

	// Copy and modify buffer.
	assert(!m_vertexBuffer.empty());
	uint32_t vertexStride = m_pRequiredVertexFormat->GetStride();

	// Create a vertex buffer sorted by collape order.
	m_progressiveMeshVertexBuffer.resize(m_vertexBuffer.size());
	for (uint32_t vertexIndex = 0U; vertexIndex < m_currentVertexCount; ++vertexIndex)
	{
		std::memcpy(m_progressiveMeshVertexBuffer.data() + m_permutation[vertexIndex] * vertexStride, m_vertexBuffer.data() + vertexIndex * vertexStride, vertexStride);
	}

	// After sorting vertex buffer, modify index buffer accordingly.
	const bool useU16Index = m_totalVertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
	m_progressiveMeshIndexBuffer.resize(m_indexBuffer.size());

	if (useU16Index)
	{
		for (uint32_t indexIndex = 0U; indexIndex < m_currentPolygonCount * 3U; ++indexIndex)
		{
			auto* pIndexData = reinterpret_cast<uint16_t*>(m_indexBuffer.data() + indexIndex * indexTypeSize);
			uint16_t index = *pIndexData;
			std::memcpy(m_progressiveMeshIndexBuffer.data() + indexIndex * indexTypeSize, &m_permutation[index], indexTypeSize);
		}
	}
	else
	{
		for (uint32_t indexIndex = 0U; indexIndex < m_currentPolygonCount * 3U; ++indexIndex)
		{
			auto* pIndexData = reinterpret_cast<uint32_t*>(m_indexBuffer.data() + indexIndex * indexTypeSize);
			uint32_t index = *pIndexData;
			std::memcpy(m_progressiveMeshIndexBuffer.data() + indexIndex * indexTypeSize, &m_permutation[index], indexTypeSize);
		}
	}

	// Submit
	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, m_pRequiredVertexFormat->GetVertexLayout());
	const bgfx::Memory* pVertexBufferRef = bgfx::makeRef(m_progressiveMeshVertexBuffer.data(), static_cast<uint32_t>(m_progressiveMeshVertexBuffer.size()));
	bgfx::VertexBufferHandle vertexBufferHandle = bgfx::createVertexBuffer(pVertexBufferRef, vertexLayout);
	assert(bgfx::isValid(vertexBufferHandle));
	m_progressiveMeshVertexBufferHandle = vertexBufferHandle.idx;

	m_progressiveMeshIndexBufferHandle = SubmitIndexBuffer<IndexBufferType::Dynamic>(m_progressiveMeshIndexBuffer, useU16Index);
}

void StaticMeshComponent::UpdateProgressiveMeshData()
{
	uint32_t lodVertexCount = static_cast<uint32_t>(m_progressiveMeshLODPercent * m_totalVertexCount);
	if (lodVertexCount == m_currentVertexCount)
	{
		return;
	}

	// update vertex used count
	m_currentVertexCount = lodVertexCount;

	// update index buffer
	uint32_t polygonCount = 0;
	const bool useU16Index = m_totalVertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
	if (useU16Index)
	{
		uint16_t* pIndexBuffer = reinterpret_cast<uint16_t*>(m_progressiveMeshIndexBuffer.data());
		for (uint32_t polygonIndex = 0U; polygonIndex < m_totalPolygonCount; ++polygonIndex)
		{
			uint16_t polygon[3];

			uint32_t startIndex = polygonIndex * 3;
			for (uint32_t ii = 0U; ii < 3U; ++ii)
			{
				uint16_t index = pIndexBuffer[startIndex + ii];
				while (index > lodVertexCount)
				{
					index = m_map[index];
				}

				polygon[ii] = index;
			}

			if (polygon[0] != polygon[1] &&
				polygon[0] != polygon[2] &&
				polygon[1] != polygon[2])
			{
				std::memcpy(pIndexBuffer + startIndex, polygon, 3 * indexTypeSize);
				++polygonCount;
			}
		}
	}
	else
	{
		assert("support u32 in generic way.");
	}

	m_currentPolygonCount = polygonCount;
	bgfx::update(bgfx::DynamicIndexBufferHandle{ m_progressiveMeshIndexBufferHandle }, 0U, bgfx::makeRef(m_progressiveMeshIndexBuffer.data(), static_cast<uint32_t>(m_progressiveMeshIndexBuffer.size())));
}

#endif

}