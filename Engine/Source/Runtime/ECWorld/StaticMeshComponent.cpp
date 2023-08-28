#include "StaticMeshComponent.h"

#include "ECWorld/World.h"
#include "Log/Log.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/VertexFormat.h"

#include <bgfx/bgfx.h>

#include <optional>

namespace engine
{

void StaticMeshComponent::Reset()
{
	m_pMeshData = nullptr;
	m_pRequiredVertexFormat = nullptr;

	m_vertexBuffer.clear();
	m_vertexBufferHandle = UINT16_MAX;

	m_indexBuffer.clear();
	m_indexBufferHandle = UINT16_MAX;
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

	uint32_t currentDataSize = 0U;
	auto currentDataPtr = m_vertexBuffer.data();

	auto FillVertexBuffer = [&currentDataPtr, &currentDataSize](const void* pData, uint32_t dataSize)
	{
		std::memcpy(&currentDataPtr[currentDataSize], pData, dataSize);
		currentDataSize += dataSize;
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

	// Create vertex buffer.
	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, m_pRequiredVertexFormat->GetVertexLayout());
	const bgfx::Memory* pVertexBufferRef = bgfx::makeRef(m_vertexBuffer.data(), static_cast<uint32_t>(m_vertexBuffer.size()));
	bgfx::VertexBufferHandle vertexBufferHandle = bgfx::createVertexBuffer(pVertexBufferRef, vertexLayout);
	assert(bgfx::isValid(vertexBufferHandle));
	m_vertexBufferHandle = vertexBufferHandle.idx;

	// Fill index buffer data.
	bool useU16Index = vertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
	m_indexBuffer.resize(m_pMeshData->GetPolygonCount() * 3 * indexTypeSize);

	currentDataSize = 0U;
	currentDataPtr = m_indexBuffer.data();
	auto FillIndexBuffer = [&currentDataPtr, &currentDataSize](const void* pData, uint32_t dataSize)
	{
		std::memcpy(&currentDataPtr[currentDataSize], pData, dataSize);
		currentDataSize += dataSize;
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

	// Create index buffer.
	const bgfx::Memory* pIndexBufferRef = bgfx::makeRef(m_indexBuffer.data(), static_cast<uint32_t>(m_indexBuffer.size()));
	bgfx::IndexBufferHandle indexBufferHandle = bgfx::createIndexBuffer(pIndexBufferRef, useU16Index ? 0U : BGFX_BUFFER_INDEX32);
	assert(bgfx::isValid(indexBufferHandle));
	m_indexBufferHandle = indexBufferHandle.idx;
}

}