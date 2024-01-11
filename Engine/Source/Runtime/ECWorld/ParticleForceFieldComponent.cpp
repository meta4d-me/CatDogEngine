#include "ParticleForceFieldComponent.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/Types.h"
#include "Scene/VertexFormat.h"

namespace engine
{
void ParticleForceFieldComponent::Build()
{
	cd::VertexFormat vertexFormat;
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);

	const uint32_t vertexCount = 8;
	std::vector<cd::Point> vertexArray
	{
		cd::Point{-m_forcefieldRange.x(), -m_forcefieldRange.y(), m_forcefieldRange.z()},
			cd::Point{m_forcefieldRange.x(), -m_forcefieldRange.y(), m_forcefieldRange.z()},
			cd::Point{m_forcefieldRange.x(), m_forcefieldRange.y(), m_forcefieldRange.z()},
			cd::Point{-m_forcefieldRange.x(), m_forcefieldRange.y(), m_forcefieldRange.z()},
			cd::Point{-m_forcefieldRange.x(), -m_forcefieldRange.y(), -m_forcefieldRange.z()},
			cd::Point{m_forcefieldRange.x(), -m_forcefieldRange.y(), -m_forcefieldRange.z()},
			cd::Point{m_forcefieldRange.x(), m_forcefieldRange.y(), -m_forcefieldRange.z()},
			cd::Point{-m_forcefieldRange.x(), m_forcefieldRange.y(), -m_forcefieldRange.z()},
	};
	m_vertexBuffer.resize(vertexCount * vertexFormat.GetStride());
	uint32_t currentDataSize = 0U;
	auto currentDataPtr = m_vertexBuffer.data();
	for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
	{
		//position
		const cd::Point& position = vertexArray[vertexIndex];
		constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
		std::memcpy(&currentDataPtr[currentDataSize], position.begin(), posDataSize);
		currentDataSize += posDataSize;
	}

	size_t indexTypeSize = sizeof(uint16_t);
	m_indexBuffer.resize(24 * indexTypeSize);
	currentDataSize = 0U;
	currentDataPtr = m_indexBuffer.data();

	std::vector<uint16_t> indexes =
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		4, 5,
		5, 6,
		6, 7,
		7, 4,

		0, 4,
		1, 5,
		2, 6,
		3, 7
	};

	for (const auto& index : indexes)
	{
		std::memcpy(&currentDataPtr[currentDataSize], &index, indexTypeSize);
		currentDataSize += static_cast<uint32_t>(indexTypeSize);
	}

	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexLayout());
	m_vertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(m_vertexBuffer.data(), static_cast<uint32_t>(m_vertexBuffer.size())), vertexLayout).idx;
	m_indexBufferHandle = bgfx::createIndexBuffer(bgfx::makeRef(m_indexBuffer.data(), static_cast<uint32_t>(m_indexBuffer.size())), 0U).idx;
}

bool ParticleForceFieldComponent::IfWithinTheRange(ForceFieldType type, float m_startRange, float m_endRange)
{
	return false;
}

}
