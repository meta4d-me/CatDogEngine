#include "ParticleEmitterComponent.h"
#include "Scene/VertexFormat.h"
#include "Rendering/Utility/VertexLayoutUtility.h"

void engine::ParticleEmitterComponent::Build()
{
	cd::VertexFormat	 vertexFormat; 
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Color, cd::AttributeValueType::Float, 3);

	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexLayout());

	size_t vertexCount = m_particleSystem.GetMaxCount();

	size_t vertexBufferSize = vertexCount * (sizeof(cd::Vec3f) + sizeof(cd::Vec3f));

	m_particleVertexBuffer.resize(vertexBufferSize);

	uint32_t currentDataSize = 0U;
	auto currentDataPtr = m_particleVertexBuffer.data();

	for (int i = 0; i < m_particleSystem.GetMaxCount(); i++)
	{
		std::memcpy(&currentDataPtr[currentDataSize], &m_particleSystem.GetPos(i), sizeof(cd::Vec3f));
		currentDataSize += sizeof(cd::Vec3f);

		std::memcpy(&currentDataPtr[currentDataSize], &m_particleSystem.GetColor(i), sizeof(cd::Vec3f));
		currentDataSize += sizeof(cd::Vec3f);
	}

/*
* indexBuffer
*/
	size_t indexTypeSize = sizeof(uint16_t);
	m_particleIndexBuffer.resize(3 * m_particleSystem.GetMaxCount() * indexTypeSize);
	currentDataSize = 0U;
	currentDataPtr = m_particleIndexBuffer.data();

	std::vector<uint16_t> indexes;
	for (uint16_t i = 0; i < m_particleSystem.GetMaxCount(); i++)
	{
		uint16_t vertexIndex = static_cast<uint16_t>(i * 3);
		indexes.push_back(vertexIndex);
		indexes.push_back(vertexIndex + 1);
		indexes.push_back(vertexIndex + 2);
	} 

	for (const auto& index : indexes)
	{
		std::memcpy(&currentDataPtr[currentDataSize], &index, indexTypeSize);
		currentDataSize += static_cast<uint32_t>(indexTypeSize);
	}

	m_particleVBH = bgfx::createVertexBuffer(bgfx::makeRef(m_particleVertexBuffer.data(), static_cast<uint32_t>(m_particleVertexBuffer.size())), vertexLayout).idx;
	m_particleIBH = bgfx::createIndexBuffer(bgfx::makeRef(m_particleIndexBuffer.data(), static_cast<uint32_t>(m_particleIndexBuffer.size())), 0U).idx;
}
