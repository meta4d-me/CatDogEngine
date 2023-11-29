#include "ParticleEmitterComponent.h"
#include "Rendering/Utility/VertexLayoutUtility.h"

#include <limits>
void engine::ParticleEmitterComponent::Build()
{
	//cd::VertexFormat	 vertexFormat;
	//vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);
	//vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Color, cd::AttributeValueType::Float, 4);
	//vertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::AttributeValueType::Float, 2);
	//bgfx::VertexLayout vertexLayout;
	//VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexLayout());
	PaddingVertexBuffer();
	PaddingIndexBuffer();

	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, m_pRequiredVertexFormat->GetVertexLayout());
	m_particleVertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(m_particleVertexBuffer.data(), static_cast<uint32_t>(m_particleVertexBuffer.size())), vertexLayout).idx;
	m_particleIndexBufferHandle = bgfx::createIndexBuffer(bgfx::makeRef(m_particleIndexBuffer.data(), static_cast<uint32_t>(m_particleIndexBuffer.size())), 0U).idx;
}

void engine::ParticleEmitterComponent::PaddingVertexBuffer()
{
	const bool containsPosition = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Position);
	const bool containsColor = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Color);
	const bool containsUV = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::UV);
	//vertexbuffer
	if (m_emitterparticletype == engine::ParticleType::Sprite)
	{
		size_t vertexCount = m_particleSystem.GetMaxCount();
		const uint32_t vertexFormatStride = m_pRequiredVertexFormat->GetStride();

		m_particleVertexBuffer.resize(vertexCount * vertexFormatStride);

		uint32_t currentDataSize = 0U;
		auto currentDataPtr = m_particleVertexBuffer.data();

		for (int i = 0; i < m_particleSystem.GetMaxCount(); i++)
		{
			if (containsPosition)
			{
				std::memcpy(&currentDataPtr[currentDataSize], &m_particleSystem.GetPos(i), sizeof(cd::Point));
				currentDataSize += sizeof(cd::Point);
			}

			if (containsColor)
			{
				std::memcpy(&currentDataPtr[currentDataSize], &m_particleSystem.GetColor(i), sizeof(cd::Color));
				currentDataSize += sizeof(cd::Color);
			}

			if (containsUV)
			{
				std::memcpy(&currentDataPtr[currentDataSize], &m_particleSystem.GetTexture_uv(i), sizeof(cd::UV));
				currentDataSize += sizeof(cd::UV);
			}
		}
	}
	else if (m_emitterparticletype == engine::ParticleType::Ribbon)
	{

	}
	else if (m_emitterparticletype == engine::ParticleType::Track)
	{

	}
	else if (m_emitterparticletype == engine::ParticleType::Ring)
	{

	}
	else if (m_emitterparticletype == engine::ParticleType::Model)
	{

	}

}

void engine::ParticleEmitterComponent::PaddingIndexBuffer()
{
	/*
* indexBuffer
*/
	if (m_emitterparticletype == engine::ParticleType::Sprite)
	{
		const bool useU16Index = static_cast<uint32_t>(m_particleSystem.GetMaxCount()) <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
		const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
		int indexCount = 6;
		const uint32_t indicesCount = m_particleSystem.GetMaxCount() / engine::ParticleTypeVertexCount::SpriteVertexCount * indexCount;
		m_particleIndexBuffer.resize(indicesCount * indexTypeSize);
		///
	/*	size_t indexTypeSize = sizeof(uint16_t);
		m_particleIndexBuffer.resize(m_particleSystem.GetMaxCount() / 4 * 6 * indexTypeSize);*/
		uint32_t currentDataSize = 0U;
		auto currentDataPtr = m_particleIndexBuffer.data();

		std::vector<uint16_t> indexes;
		for (uint16_t i = 0; i < m_particleSystem.GetMaxCount(); i += engine::ParticleTypeVertexCount::SpriteVertexCount)
		{
			uint16_t vertexIndex = static_cast<uint16_t>(i);
			indexes.push_back(vertexIndex);
			indexes.push_back(vertexIndex + 1);
			indexes.push_back(vertexIndex + 2);
			indexes.push_back(vertexIndex);
			indexes.push_back(vertexIndex + 2);
			indexes.push_back(vertexIndex + 3);
		}

		for (const auto& index : indexes)
		{
			std::memcpy(&currentDataPtr[currentDataSize], &index, indexTypeSize);
			currentDataSize += static_cast<uint32_t>(indexTypeSize);
		}
	}
	else if (m_emitterparticletype == engine::ParticleType::Ribbon)
	{

	}
	else if (m_emitterparticletype == engine::ParticleType::Track)
	{

	}
	else if (m_emitterparticletype == engine::ParticleType::Ring)
	{

	}
	else if (m_emitterparticletype == engine::ParticleType::Model)
	{

	}
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
