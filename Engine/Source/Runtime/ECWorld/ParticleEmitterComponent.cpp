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
	//m_particleVertexBuffer.clear();
	//m_particleVertexBuffer.insert(m_particleVertexBuffer.end(), m_particlePool.GetRenderDataBuffer().begin(), m_particlePool.GetRenderDataBuffer().end());

		const bool containsPosition = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Position);
	const bool containsColor = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Color);
	const bool containsUV = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::UV);
	//vertexbuffer
	if (m_emitterparticletype == engine::ParticleType::Sprite)
	{
		const int MAX_VERTEX_COUNT = 300;
		size_t vertexCount = MAX_VERTEX_COUNT;
		const uint32_t vertexFormatStride = m_pRequiredVertexFormat->GetStride();

		m_particleVertexBuffer.resize(vertexCount * vertexFormatStride);

		uint32_t currentDataSize = 0U;
		auto currentDataPtr = m_particleVertexBuffer.data();

		VertexData vertexDataBuffer[MAX_VERTEX_COUNT];
		for (int i = 0; i < MAX_VERTEX_COUNT; i += engine::ParticleTypeVertexCount::SpriteVertexCount)
		{
			vertexDataBuffer[i] = { cd::Vec3f(-1.0f,-1.0f,0.0f),cd::Vec4f(1.0f,1.0f,1.0f,1.0f),cd::Vec2f(1.0f,1.0f) };
			vertexDataBuffer[i + 1] = { cd::Vec3f(1.0f,-1.0f,0.0f),cd::Vec4f(1.0f,1.0f,1.0f,1.0f),cd::Vec2f(0.0f,1.0f) };
			vertexDataBuffer[i + 2] = { cd::Vec3f(1.0f,1.0f,0.0f),cd::Vec4f(1.0f,1.0f,1.0f,1.0f),cd::Vec2f(0.0f,0.0f) };
			vertexDataBuffer[i + 3] = { cd::Vec3f(-1.0f,1.0f,0.0f),cd::Vec4f(1.0f,1.0f,1.0f,1.0f),cd::Vec2f(1.0f,0.0f) };
		}

		for (int i = 0; i < MAX_VERTEX_COUNT; ++i)
		{
			if (containsPosition)
			{
				std::memcpy(&currentDataPtr[currentDataSize], &vertexDataBuffer[i].pos, sizeof(cd::Point));
				currentDataSize += sizeof(cd::Point);
			}

			if (containsColor)
			{
				std::memcpy(&currentDataPtr[currentDataSize], &vertexDataBuffer[i].color, sizeof(cd::Color));
				currentDataSize += sizeof(cd::Color);
			}

			if (containsUV)
			{
				std::memcpy(&currentDataPtr[currentDataSize], &vertexDataBuffer[i].uv, sizeof(cd::UV));
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
	 //   m_particleIndexBuffer.clear();
		//const bool useU16Index = static_cast<uint32_t>(ParticleTypeVertexCount::SpriteVertexCount) <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
		//const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
		//const uint32_t indicesCount = 6;
		//m_particleIndexBuffer.resize(indicesCount * indexTypeSize);
		//m_particleIndexBuffer.push_back(static_cast<std::byte>(0));
		//m_particleIndexBuffer.push_back(static_cast<std::byte>(1));
		//m_particleIndexBuffer.push_back(static_cast<std::byte>(2));
		//m_particleIndexBuffer.push_back(static_cast<std::byte>(2));
		//m_particleIndexBuffer.push_back(static_cast<std::byte>(1));
		//m_particleIndexBuffer.push_back(static_cast<std::byte>(3));
	/*
* indexBuffer
*/
	if (m_emitterparticletype == engine::ParticleType::Sprite)
	{
		const bool useU16Index = static_cast<uint32_t>(ParticleTypeVertexCount::SpriteVertexCount) <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
		const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
		const int MAX_VERTEX_COUNT = 300;
		int indexCount = MAX_VERTEX_COUNT;
		const uint32_t indicesCount = MAX_VERTEX_COUNT * 6;
		m_particleIndexBuffer.resize(indicesCount * indexTypeSize);
		///
	/*	size_t indexTypeSize = sizeof(uint16_t);
		m_particleIndexBuffer.resize(m_particleSystem.GetMaxCount() / 4 * 6 * indexTypeSize);*/
		uint32_t currentDataSize = 0U;
		auto currentDataPtr = m_particleIndexBuffer.data();

		std::vector<uint16_t> indexes;
		for (uint16_t i = 0; i < MAX_VERTEX_COUNT; i += ParticleTypeVertexCount::SpriteVertexCount)
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
