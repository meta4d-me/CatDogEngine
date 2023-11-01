#include "ParticleEmitterComponent.h"
#include "Scene/VertexFormat.h"
#include "Rendering/Utility/VertexLayoutUtility.h"

void engine::ParticleEmitterComponent::Build()
{
	cd::VertexFormat	 vertexFormat;
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Color, cd::AttributeValueType::Float, 4);
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::AttributeValueType::Float, 2);
	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexLayout());
	
	PaddingVertexBuffer();
	PaddingIndexBuffer();

	m_particleVBH = bgfx::createVertexBuffer(bgfx::makeRef(m_particleVertexBuffer.data(), static_cast<uint32_t>(m_particleVertexBuffer.size())), vertexLayout).idx;
	m_particleIBH = bgfx::createIndexBuffer(bgfx::makeRef(m_particleIndexBuffer.data(), static_cast<uint32_t>(m_particleIndexBuffer.size())), 0U).idx;
}

//void engine::ParticleEmitterComponent::UpdateBuffer()
//{
//	//bgfx::update(vbh, 0, bgfx::makeRef(vertexBuffer.data(), vertexBuffer.size()));
//	PaddingVertexBuffer();
//	PaddingIndexBuffer();
//
//	bgfx::update(bgfx::DynamicVertexBufferHandle{ GetParticleVBH() }, 0 , bgfx::makeRef(m_particleVertexBuffer.data(), static_cast<uint32_t>(m_particleVertexBuffer.size())));
//	bgfx::update(bgfx::DynamicIndexBufferHandle{GetParticleIBH()}, 0, bgfx::makeRef(m_particleIndexBuffer.data(), static_cast<uint32_t>(m_particleIndexBuffer.size())));
//}

void engine::ParticleEmitterComponent::PaddingVertexBuffer()
{
	//vertexbuffer

	size_t vertexCount = m_particleSystem.GetMaxCount();

	size_t vertexBufferSize = vertexCount * (sizeof(cd::Vec3f) + sizeof(cd::Vec4f)+sizeof(float)*2);

	m_particleVertexBuffer.resize(vertexBufferSize);

	uint32_t currentDataSize = 0U;
	auto currentDataPtr = m_particleVertexBuffer.data();

	for (int i = 0; i < m_particleSystem.GetMaxCount(); i++)
	{
		std::memcpy(&currentDataPtr[currentDataSize], &m_particleSystem.GetPos(i), sizeof(cd::Vec3f));
		currentDataSize += sizeof(cd::Vec3f);

		std::memcpy(&currentDataPtr[currentDataSize], &m_particleSystem.GetColor(i), sizeof(cd::Vec4f));
		currentDataSize += sizeof(cd::Vec4f);
		
		std::memcpy(&currentDataPtr[currentDataSize], &m_particleSystem.GetTexture_u(i), sizeof(float));
		currentDataSize += sizeof(float);

		std::memcpy(&currentDataPtr[currentDataSize], &m_particleSystem.GetTexture_v(i), sizeof(float));
		currentDataSize += sizeof(float);
	}
}

void engine::ParticleEmitterComponent::PaddingIndexBuffer()
{
	/*
* indexBuffer
*/
	size_t indexTypeSize = sizeof(uint16_t);
	m_particleIndexBuffer.resize(m_particleSystem.GetMaxCount()/4* 6 * indexTypeSize);
	uint32_t currentDataSize = 0U;
	auto currentDataPtr = m_particleIndexBuffer.data();

	std::vector<uint16_t> indexes;
	for (uint16_t i = 0; i < m_particleSystem.GetMaxCount(); i += 4)
	{
		uint16_t vertexIndex = static_cast<uint16_t>(i);
		indexes.push_back(vertexIndex);
		indexes.push_back(vertexIndex+1);
		indexes.push_back(vertexIndex+2);
		indexes.push_back(vertexIndex);
		indexes.push_back(vertexIndex+2);
		indexes.push_back(vertexIndex+3);

	}

	for (const auto& index : indexes)
	{
		std::memcpy(&currentDataPtr[currentDataSize], &index, indexTypeSize);
		currentDataSize += static_cast<uint32_t>(indexTypeSize);
	}
}