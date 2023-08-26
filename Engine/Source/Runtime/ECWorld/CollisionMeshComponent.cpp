#include "CollisionMeshComponent.h"

#include "Math/MeshGenerator.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/Mesh.h"
#include "Scene/VertexFormat.h"

namespace engine
{

void CollisionMeshComponent::Reset()
{
	m_aabb.Clear();
	m_aabbVertexBuffer.clear();
	m_aabbVBH = UINT16_MAX;

	m_aabbIndexBuffer.clear();
	m_aabbIBH = UINT16_MAX;
}

void CollisionMeshComponent::Build()
{
	if (m_aabb.IsEmpty())
	{
		return;
	}

	cd::VertexFormat vertexFormat;
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);

	//vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Color, cd::AttributeValueType::Float, 4);
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
		//const cd::Vec4f& barycentricCoordinates = meshData.GetVertexColor(0U, vertexIndex);
		//constexpr uint32_t bcDataSize = cd::Vec4f::Size * sizeof(cd::Vec4f::ValueType);
		//std::memcpy(&currentDataPtr[currentDataSize], barycentricCoordinates.Begin(), bcDataSize);
		//currentDataSize += bcDataSize;
	}

	// AABB should always use u16 index type.
	size_t indexTypeSize = sizeof(uint16_t);
	m_aabbIndexBuffer.resize(12 * 2 * indexTypeSize);
	assert(meshData.GetPolygonCount() <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()));
	currentDataSize = 0U;
	currentDataPtr = m_aabbIndexBuffer.data();

	std::vector<uint16_t> indexes =
	{
		0U,1U,1U,5U,5U,4U,4U,0U,
		0U,2U,1U,3U,5U,7U,4U,6U,
		2U,3U,3U,7U,7U,6U,6U,2U
	};

	for (const auto& index : indexes)
	{
		std::memcpy(&currentDataPtr[currentDataSize], &index, indexTypeSize);
		currentDataSize += static_cast<uint32_t>(indexTypeSize);
	}

	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexLayout());
	m_aabbVBH = bgfx::createVertexBuffer(bgfx::makeRef(m_aabbVertexBuffer.data(), static_cast<uint32_t>(m_aabbVertexBuffer.size())), vertexLayout).idx;
	m_aabbIBH = bgfx::createIndexBuffer(bgfx::makeRef(m_aabbIndexBuffer.data(), static_cast<uint32_t>(m_aabbIndexBuffer.size())), 0U).idx;
}

}