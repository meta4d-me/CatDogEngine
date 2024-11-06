#include "Log/Log.h"
#include "ParticleRibbonComponent.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Utilities/MeshUtils.hpp"

#include <limits>

namespace engine
{
void ParticleRibbonComponent::Build()
{
	PaddingRibbonVertexBuffer();
	PaddingRibbonIndexBuffer();
}

void ParticleRibbonComponent::PaddingRibbonVertexBuffer()
{
	//vertexbuffer
	constexpr int meshVertexCount = Particle::GetMeshVertexCount<ParticleType::Ribbon>();
	//75 is from particle's MaxCount,There only padding once.
	const int MAX_VERTEX_COUNT = 75 * meshVertexCount;
	size_t vertexCount = MAX_VERTEX_COUNT;
	uint32_t csVertexCount = MAX_VERTEX_COUNT;
	//prePos Vertex format/layout
	cd::VertexFormat m_pRibbonPrePosVertexFormat;
	m_pRibbonPrePosVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	m_pRibbonPrePosVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::BoneWeight, cd::AttributeValueType::Float, 1U);
	bgfx::VertexLayout prePosLayout;
	VertexLayoutUtility::CreateVertexLayout(prePosLayout, m_pRibbonPrePosVertexFormat.GetVertexAttributeLayouts());
	//remain Vertex format/layout
	cd::VertexFormat m_pRibbonRemainVertexFormat;
	m_pRibbonRemainVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Color, cd::GetAttributeValueType<cd::Color::ValueType>(), cd::Color::Size);
	m_pRibbonRemainVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	bgfx::VertexLayout color_UV_Layout;
	VertexLayoutUtility::CreateVertexLayout(color_UV_Layout, m_pRibbonRemainVertexFormat.GetVertexAttributeLayouts());

	const uint32_t remainVertexFormatStride = m_pRibbonRemainVertexFormat.GetStride();

	m_ribbonParticleRemainVertexBuffer.resize(vertexCount * remainVertexFormatStride);

	uint32_t currentRemainDataSize = 0U;
	auto currentRemainDataPtr = m_ribbonParticleRemainVertexBuffer.data();

	std::vector<VertexData> vertexDataBuffer;
	vertexDataBuffer.resize(MAX_VERTEX_COUNT);
	// pos color uv
	// only a picture now
	for (int i = 0; i < MAX_VERTEX_COUNT; i += meshVertexCount)
	{
		vertexDataBuffer[i] = { cd::Vec3f(0.0f,1.0f,0.0f),cd::Vec4f(1.0f,1.0f,1.0f,1.0f),cd::Vec2f(0.5f,0.5f) };
		vertexDataBuffer[i + 1] = { cd::Vec3f(0.0f,-1.0f,0.0f),cd::Vec4f(1.0f,1.0f,1.0f,1.0f),cd::Vec2f(0.5f,0.5f) };
	}

	for (int i = 0; i < MAX_VERTEX_COUNT; ++i)
	{
		std::memcpy(&currentRemainDataPtr[currentRemainDataSize], &vertexDataBuffer[i].color, sizeof(cd::Color));
		currentRemainDataSize += sizeof(cd::Color);
		std::memcpy(&currentRemainDataPtr[currentRemainDataSize], &vertexDataBuffer[i].uv, sizeof(cd::UV));
		currentRemainDataSize += sizeof(cd::UV);
	}
	m_ribbonParticlePrePosVertexBufferHandle = bgfx::createDynamicVertexBuffer(csVertexCount, prePosLayout, BGFX_BUFFER_COMPUTE_READ_WRITE).idx;
	const bgfx::Memory* pRibbonParticleRemainVBRef = bgfx::makeRef(m_ribbonParticleRemainVertexBuffer.data(), static_cast<uint32_t>(m_ribbonParticleRemainVertexBuffer.size()));
	m_ribbonParticleRemainVertexBufferHandle = bgfx::createVertexBuffer(pRibbonParticleRemainVBRef, color_UV_Layout).idx;
}

void ParticleRibbonComponent::PaddingRibbonIndexBuffer()
{
	constexpr int meshVertexCount = Particle::GetMeshVertexCount<ParticleType::Ribbon>();;
	const bool useU16Index = meshVertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
	//TODO: 75 is from particle's MaxCount There only padding once.
	const int MAX_VERTEX_COUNT = (75 - 1) * meshVertexCount;
	int indexCountForOneRibbon = 6;
	const uint32_t indicesCount = MAX_VERTEX_COUNT / meshVertexCount * indexCountForOneRibbon;
	m_ribbonParticleIndexBuffer.resize(indicesCount * indexTypeSize);
	///
/*	size_t indexTypeSize = sizeof(uint16_t);
	m_particleIndexBuffer.resize(m_particleSystem.GetMaxCount() / 4 * 6 * indexTyp
	eSize);*/
	uint32_t currentDataSize = 0U;
	auto currentDataPtr = m_ribbonParticleIndexBuffer.data();

	std::vector<uint16_t> indexes;

	for (uint16_t i = 0; i < MAX_VERTEX_COUNT; i += meshVertexCount)
	{
		uint16_t vertexIndex = static_cast<uint16_t>(i);
		indexes.push_back(vertexIndex);
		indexes.push_back(vertexIndex + 1);
		indexes.push_back(vertexIndex + 2);

		indexes.push_back(vertexIndex + 2);
		indexes.push_back(vertexIndex + 1);
		indexes.push_back(vertexIndex + 3);
	}

	for (const auto& index : indexes)
	{
		std::memcpy(&currentDataPtr[currentDataSize], &index, indexTypeSize);
		currentDataSize += static_cast<uint32_t>(indexTypeSize);
	}
	m_ribbonParticleIndexBufferHandle = bgfx::createIndexBuffer(bgfx::makeRef(m_ribbonParticleIndexBuffer.data(), static_cast<uint32_t>(m_ribbonParticleIndexBuffer.size())), 0U).idx;
}

}