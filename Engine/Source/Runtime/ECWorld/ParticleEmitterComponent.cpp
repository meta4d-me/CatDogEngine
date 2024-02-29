#include "Log/Log.h"
#include "ParticleEmitterComponent.h"

#include "Rendering/Utility/VertexLayoutUtility.h"

#include <limits>

void engine::ParticleEmitterComponent::Build()
{
	if (m_pMeshData == nullptr)
	{
		PaddingVertexBuffer();
		PaddingIndexBuffer();
	}
	else
	{
		ParseMeshVertexBuffer();
		ParseMeshIndexBuffer();
	}

	BuildParticleShape();

	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, m_pRequiredVertexFormat->GetVertexAttributeLayouts());
	m_particleVertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(m_particleVertexBuffer.data(), static_cast<uint32_t>(m_particleVertexBuffer.size())), vertexLayout).idx;
	m_particleIndexBufferHandle = bgfx::createIndexBuffer(bgfx::makeRef(m_particleIndexBuffer.data(), static_cast<uint32_t>(m_particleIndexBuffer.size())), 0U).idx;
}

void engine::ParticleEmitterComponent::PaddingVertexBuffer()
{
	//m_particleVertexBuffer.clear();
	//m_particleVertexBuffer.insert(m_particleVertexBuffer.end(), m_particlePool.GetRenderDataBuffer().begin(), m_particlePool.GetRenderDataBuffer().end());
	m_particleVertexBuffer.clear();

	const bool containsPosition = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Position);
	const bool containsColor = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Color);
	const bool containsUV = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::UV);
	//vertexbuffer
	if (m_emitterParticleType == engine::ParticleType::Sprite)
	{
		const int MAX_VERTEX_COUNT = m_particlePool.GetParticleMaxCount() * engine::ParticleTypeVertexCount::SpriteVertexCount;
		size_t vertexCount = MAX_VERTEX_COUNT;
		const uint32_t vertexFormatStride = m_pRequiredVertexFormat->GetStride();

		m_particleVertexBuffer.resize(vertexCount * vertexFormatStride);

		uint32_t currentDataSize = 0U;
		auto currentDataPtr = m_particleVertexBuffer.data();

		std::vector<VertexData> vertexDataBuffer;
		vertexDataBuffer.resize(MAX_VERTEX_COUNT);
		// pos color uv
		// only a picture now
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
	else if (m_emitterParticleType == engine::ParticleType::Ribbon)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Track)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Ring)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Model)
	{

	}

}

void engine::ParticleEmitterComponent::PaddingIndexBuffer()
{
	/*
* indexBuffer
*/	
	m_particleIndexBuffer.clear();
	if (m_emitterParticleType == engine::ParticleType::Sprite)
	{
		const bool useU16Index = static_cast<uint32_t>(ParticleTypeVertexCount::SpriteVertexCount) <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
		const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
		const int MAX_VERTEX_COUNT = m_particlePool.GetParticleMaxCount() * engine::ParticleTypeVertexCount::SpriteVertexCount;
		int indexCountForOneSprite = 6;
		const uint32_t indicesCount = MAX_VERTEX_COUNT / engine::ParticleTypeVertexCount::SpriteVertexCount * indexCountForOneSprite;
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
	else if (m_emitterParticleType == engine::ParticleType::Ribbon)
	{
	
	}
	else if (m_emitterParticleType == engine::ParticleType::Track)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Ring)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Model)
	{

	}
}

void engine::ParticleEmitterComponent::ParseMeshVertexBuffer()
{
	CD_ASSERT(m_pMeshData && m_pRequiredVertexFormat, "Input data is not ready.");
		
	if (!m_pMeshData->GetVertexFormat().IsCompatiableTo(*m_pRequiredVertexFormat))
	{
		CD_ERROR("Current mesh data is not compatiable to required vertex format.");
		return;
	}
	
	m_particleVertexBuffer.clear();
	//TODO: if particle is mesh , is there should be to change the mesh data?
	m_spawnCount = m_pMeshData->GetPolygonCount()/2;

	const bool containsPosition = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Position);
	const bool containsColor = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::Color);
	const bool containsUV = m_pRequiredVertexFormat->Contains(cd::VertexAttributeType::UV);

	if (m_emitterParticleType == engine::ParticleType::Sprite)
	{
		const int MAX_VERTEX_COUNT = m_pMeshData->GetVertexCount();
		size_t vertexCount = MAX_VERTEX_COUNT;
		const uint32_t vertexFormatStride = m_pRequiredVertexFormat->GetStride();

		m_particleVertexBuffer.resize(vertexCount * vertexFormatStride);

		uint32_t currentDataSize = 0U;
		auto currentDataPtr = m_particleVertexBuffer.data();

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
				FillVertexBuffer(m_pMeshData->GetVertexPosition(vertexIndex).begin(), dataSize);
			}

			if (containsColor)
			{
				constexpr uint32_t dataSize = cd::Color::Size * sizeof(cd::Color::ValueType);
				FillVertexBuffer(m_pMeshData->GetVertexColor(0)[vertexIndex].begin(), dataSize);
			}

			if (containsUV)
			{
				constexpr uint32_t dataSize = cd::UV::Size * sizeof(cd::UV::ValueType);
				FillVertexBuffer(m_pMeshData->GetVertexUV(0)[vertexIndex].begin(), dataSize);
			}
		}
	}
	else if (m_emitterParticleType == engine::ParticleType::Ribbon)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Track)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Ring)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Model)
	{

	}
}

void engine::ParticleEmitterComponent::ParseMeshIndexBuffer()
{
	m_particleIndexBuffer.clear();
	if (m_emitterParticleType == engine::ParticleType::Sprite)
	{
		const bool useU16Index = static_cast<uint32_t>(ParticleTypeVertexCount::SpriteVertexCount) <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
		const uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
		const uint32_t indicesCount = m_pMeshData->GetPolygonCount()*3U;
		m_particleIndexBuffer.resize(indicesCount * indexTypeSize);

		uint32_t currentDataSize = 0U;
		auto currentDataPtr = m_particleIndexBuffer.data();

		auto FillIndexBuffer = [&currentDataPtr, &currentDataSize](const void* pData, uint32_t dataSize)
		{
			std::memcpy(&currentDataPtr[currentDataSize], pData, dataSize);
			currentDataSize += dataSize;
		};

		uint32_t polygonGroupIndex = 0U;
		for (const auto& polygon : m_pMeshData->GetPolygonGroup(polygonGroupIndex))
		{
			if (useU16Index)
			{
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
	}
	else if (m_emitterParticleType == engine::ParticleType::Ribbon)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Track)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Ring)
	{

	}
	else if (m_emitterParticleType == engine::ParticleType::Model)
	{

	}
}

void engine::ParticleEmitterComponent::BuildParticleShape()
{
	if (m_emitterShape == ParticleEmitterShape::Box)
	{
		cd::VertexFormat vertexFormat;
		vertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);

		const uint32_t vertexCount = 8;
		std::vector<cd::Point> vertexArray
		{
			cd::Point{-1.0f, -1.0f, 1.0f},
				cd::Point{1.0f, -1.0f, 1.0f},
				cd::Point{1.0f, 1.0f, 1.0f},
				cd::Point{-1.0f, 1.0f, 1.0f},
				cd::Point{-1.0f, -1.0f, -1.0f},
				cd::Point{1.0f, -1.0f, -1.0f},
				cd::Point{1.0f, 1.0f, -1.0f},
				cd::Point{-1.0f, 1.0f, -1.0f},
		};
		m_emitterShapeVertexBuffer.resize(vertexCount * vertexFormat.GetStride());
		uint32_t currentDataSize = 0U;
		auto currentDataPtr = m_emitterShapeVertexBuffer.data();
		for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			//position
			const cd::Point& position = vertexArray[vertexIndex];
			constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
			std::memcpy(&currentDataPtr[currentDataSize], position.begin(), posDataSize);
			currentDataSize += posDataSize;
		}

		size_t indexTypeSize = sizeof(uint16_t);
		m_emitterShapeIndexBuffer.resize(24 * indexTypeSize);
		currentDataSize = 0U;
		currentDataPtr = m_emitterShapeIndexBuffer.data();

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
		VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexAttributeLayouts());
		m_emitterShapeVertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(m_emitterShapeVertexBuffer.data(), static_cast<uint32_t>(m_emitterShapeVertexBuffer.size())), vertexLayout).idx;
		m_emitterShapeIndexBufferHandle = bgfx::createIndexBuffer(bgfx::makeRef(m_emitterShapeIndexBuffer.data(), static_cast<uint32_t>(m_emitterShapeIndexBuffer.size())), 0U).idx;
	}
}

void engine::ParticleEmitterComponent::RePaddingShapeBuffer()
{
	if (m_emitterShape == ParticleEmitterShape::Box)
	{
		cd::VertexFormat vertexFormat;
		vertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);

		const uint32_t vertexCount = 8;
		std::vector<cd::Point> vertexArray
		{
			cd::Point{-m_emitterShapeRange.x(), -m_emitterShapeRange.y(), m_emitterShapeRange.z()},
				cd::Point{m_emitterShapeRange.x(), -m_emitterShapeRange.y(), m_emitterShapeRange.z()},
				cd::Point{m_emitterShapeRange.x(), m_emitterShapeRange.y(), m_emitterShapeRange.z()},
				cd::Point{-m_emitterShapeRange.x(), m_emitterShapeRange.y(), m_emitterShapeRange.z()},
				cd::Point{-m_emitterShapeRange.x(), -m_emitterShapeRange.y(), -m_emitterShapeRange.z()},
				cd::Point{m_emitterShapeRange.x(), -m_emitterShapeRange.y(), -m_emitterShapeRange.z()},
				cd::Point{m_emitterShapeRange.x(), m_emitterShapeRange.y(), -m_emitterShapeRange.z()},
				cd::Point{-m_emitterShapeRange.x(), m_emitterShapeRange.y(), -m_emitterShapeRange.z()},
		};
		m_emitterShapeVertexBuffer.resize(vertexCount * vertexFormat.GetStride());
		uint32_t currentDataSize = 0U;
		auto currentDataPtr = m_emitterShapeVertexBuffer.data();
		for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			//position
			const cd::Point& position = vertexArray[vertexIndex];
			constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
			std::memcpy(&currentDataPtr[currentDataSize], position.begin(), posDataSize);
			currentDataSize += posDataSize;
		}

		size_t indexTypeSize = sizeof(uint16_t);
		m_emitterShapeIndexBuffer.resize(24 * indexTypeSize);
		currentDataSize = 0U;
		currentDataPtr = m_emitterShapeIndexBuffer.data();

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
	}
}

const std::string& engine::ParticleEmitterComponent::GetShaderProgramName() const
{
	return m_pParticleMaterialType->GetShaderSchema().GetShaderProgramName();
}

void engine::ParticleEmitterComponent::ActivateShaderFeature(ShaderFeature feature)
{
	if (ShaderFeature::DEFAULT == feature)
	{
		return;
	}

	for (const auto& conflict : m_pParticleMaterialType->GetShaderSchema().GetConflictFeatureSet(feature))
	{
		m_shaderFeatures.erase(conflict);
	}

	m_shaderFeatures.insert(cd::MoveTemp(feature));

	m_isShaderFeatureDirty = true;
}

void engine::ParticleEmitterComponent::DeactivateShaderFeature(ShaderFeature feature)
{
	m_shaderFeatures.erase(feature);

	m_isShaderFeatureDirty = true;
}

const std::string& engine::ParticleEmitterComponent::GetFeaturesCombine()
{
	if (m_isShaderFeatureDirty == false)
	{
		return m_featureCombine;
	}

	m_featureCombine = m_pParticleMaterialType->GetShaderSchema().GetFeaturesCombine(m_shaderFeatures);
	m_isShaderFeatureDirty = false;

	return m_featureCombine;
}



