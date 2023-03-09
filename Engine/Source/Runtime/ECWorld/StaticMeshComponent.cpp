#include "StaticMeshComponent.h"

#include "ECWorld/World.h"
#include "Log/Log.h"
#include "Math/MeshGenerator.h"
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

	// Debug
	m_aabb.Clear();
	m_aabbVertexBuffer.clear();
	m_aabbVBH = UINT16_MAX;

	m_aabbIndexBuffer.clear();
	m_aabbIBH = UINT16_MAX;
}

void StaticMeshComponent::BuildDebug()
{
	m_aabb = m_pMeshData->GetAABB();
	if (m_aabb.IsEmpty())
	{
		return;
	}

	cd::VertexFormat vertexFormat;
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);

	// Hack : use bitangents as barycentric coordinate...
	// TODO : Vertex attribute type can have userdata storage and map to slot enum.
	// It requires duplicated vertices so only used for debug/editor usage.
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Bitangent, cd::AttributeValueType::Float, 3);
	std::optional<cd::Mesh> optMesh = cd::MeshGenerator::Generate(cd::Box(m_aabb.Min(), m_aabb.Max()), vertexFormat);
	if (!optMesh.has_value())
	{
		return;
	}

	const cd::Mesh& meshData = optMesh.value();
	const uint32_t vertexCount = meshData.GetVertexCount();
	m_aabbVertexBuffer.resize(vertexCount * vertexFormat.GetStride());
	uint32_t currentDataSize = 0U;
	auto currentDataPtr = m_aabbVertexBuffer.data();
	for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
	{
		// position
		const cd::Point& position = meshData.GetVertexPosition(vertexIndex);
		constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
		std::memcpy(&currentDataPtr[currentDataSize], position.Begin(), posDataSize);
		currentDataSize += posDataSize;

		// barycentric
		const cd::Point& barycentricCoordinates = meshData.GetVertexBiTangent(vertexIndex);
		constexpr uint32_t bcDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
		std::memcpy(&currentDataPtr[currentDataSize], barycentricCoordinates.Begin(), bcDataSize);
		currentDataSize += bcDataSize;
	}
	
	m_aabbIndexBuffer.resize(meshData.GetPolygonCount() * cd::Polygon::Size * sizeof(cd::Polygon::ValueType));
	std::memcpy(m_aabbIndexBuffer.data(), meshData.GetPolygons().data(), m_aabbIndexBuffer.size());

	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexLayout());
	m_aabbVBH = bgfx::createVertexBuffer(bgfx::makeRef(m_aabbVertexBuffer.data(), static_cast<uint32_t>(m_aabbVertexBuffer.size())), vertexLayout).idx;
	m_aabbIBH = bgfx::createIndexBuffer(bgfx::makeRef(m_aabbIndexBuffer.data(), static_cast<uint32_t>(m_aabbIndexBuffer.size())), BGFX_BUFFER_INDEX32).idx;
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
	bgfx::VertexBufferHandle vertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(m_vertexBuffer.data(), static_cast<uint32_t>(m_vertexBuffer.size())), vertexLayout);
	assert(bgfx::isValid(vertexBufferHandle));
	m_vertexBufferHandle = vertexBufferHandle.idx;

	// Create index buffer.
	m_indexBuffer.resize(m_pMeshData->GetPolygonCount() * cd::Polygon::Size * sizeof(cd::Polygon::ValueType));
	std::memcpy(m_indexBuffer.data(), m_pMeshData->GetPolygons().data(), m_indexBuffer.size());
	bgfx::IndexBufferHandle indexBufferHandle = bgfx::createIndexBuffer(bgfx::makeRef(m_indexBuffer.data(), static_cast<uint32_t>(m_indexBuffer.size())), BGFX_BUFFER_INDEX32);
	assert(bgfx::isValid(indexBufferHandle));
	m_indexBufferHandle = indexBufferHandle.idx;

	// Build debug data.
	BuildDebug();
}

}