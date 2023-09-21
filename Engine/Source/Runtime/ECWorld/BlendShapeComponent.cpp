#include "BlendShapeComponent.h"

#include "ECWorld/World.h"
#include "Log/Log.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/VertexFormat.h"

#include <bgfx/bgfx.h>

#include <optional>

namespace engine
{
/*
void BlendShapeComponent::Reset()
{
	
}
*/
void BlendShapeComponent::Build()
{
	//
	const uint32_t meshVertexCount = m_pMesh->GetVertexCount();
	m_vertexCount = meshVertexCount;
	const uint32_t morphCount = static_cast<uint32_t>(m_pMorphs->size());
	m_weights.resize(m_pMorphs->size(), 0.2f);

	uint32_t positionSize = cd::Point::Size * sizeof(cd::Point::ValueType);
	uint32_t normalSize = cd::Direction::Size * sizeof(cd::Direction::ValueType);
	uint32_t tangentSize = cd::Direction::Size * sizeof(cd::Direction::ValueType);
	uint32_t UVSize = cd::UV::Size * sizeof(cd::UV::ValueType);
	float placeholder = 0.0f;
	uint32_t placeholderSize = sizeof(placeholder);

	//1. Morph Affected : position ,  Morph Non-Affected : normal tangent uv
	cd::VertexFormat morphAffectedVF;//Morph Affected Vertex Format
	morphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	const uint32_t morphAffectedVFStride = morphAffectedVF.GetStride();
	m_morphAffectedVB.resize(meshVertexCount * 16);
	auto morphAffectedVBDataPtr = m_morphAffectedVB.data();
	uint32_t morphAffectedVBDataSize = 0U;

	cd::VertexFormat nonMorphAffectedVF;//Morph Non-Affected Vertex Format
	nonMorphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	nonMorphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	nonMorphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	const uint32_t nonMorphAffectedVFStride = nonMorphAffectedVF.GetStride();
	m_nonMorphAffectedVB.resize(meshVertexCount * nonMorphAffectedVFStride);
	auto nonMorphAffectedDataPtr = m_nonMorphAffectedVB.data();
	uint32_t nonMorphAffectedVBDataSize = 0U;

	for (uint32_t vertexIndex = 0; vertexIndex < meshVertexCount; ++vertexIndex)
	{
		std::memcpy(&morphAffectedVBDataPtr[morphAffectedVBDataSize], m_pMesh->GetVertexPosition(vertexIndex).Begin(), positionSize);
		morphAffectedVBDataSize += positionSize;
		std::memcpy(&morphAffectedVBDataPtr[morphAffectedVBDataSize], &placeholder, placeholderSize);
		morphAffectedVBDataSize += placeholderSize;
		std::memcpy(&nonMorphAffectedDataPtr[nonMorphAffectedVBDataSize], m_pMesh->GetVertexNormal(vertexIndex).Begin(), normalSize);
		nonMorphAffectedVBDataSize += normalSize;
		std::memcpy(&nonMorphAffectedDataPtr[nonMorphAffectedVBDataSize], m_pMesh->GetVertexTangent(vertexIndex).Begin(), tangentSize);
		nonMorphAffectedVBDataSize += tangentSize;
		std::memcpy(&nonMorphAffectedDataPtr[nonMorphAffectedVBDataSize], m_pMesh->GetVertexUV(0, vertexIndex).Begin(), UVSize);
		nonMorphAffectedVBDataSize += UVSize;
	}

	bgfx::VertexLayout morphAffectedVL;
	VertexLayoutUtility::CreateVertexLayout(morphAffectedVL, morphAffectedVF.GetVertexLayout());
	const bgfx::Memory* pMorphAffectedVBRef = bgfx::makeRef(m_morphAffectedVB.data(), static_cast<uint32_t>(m_morphAffectedVB.size()));
	bgfx::VertexBufferHandle morphAffectedVBHandle = bgfx::createVertexBuffer(pMorphAffectedVBRef, morphAffectedVL, BGFX_BUFFER_COMPUTE_READ);
	assert(bgfx::isValid(morphAffectedVBHandle));
	m_morphAffectedVBHandle = morphAffectedVBHandle.idx;

	bgfx::VertexLayout nonMorphAffectedVL;
	VertexLayoutUtility::CreateVertexLayout(nonMorphAffectedVL, nonMorphAffectedVF.GetVertexLayout());
	const bgfx::Memory* pNonMorphAffectedVBRef = bgfx::makeRef(m_nonMorphAffectedVB.data(), static_cast<uint32_t>(m_nonMorphAffectedVB.size()));
	bgfx::VertexBufferHandle nonMorphAffectedVBHandle = bgfx::createVertexBuffer(pNonMorphAffectedVBRef, nonMorphAffectedVL);
	assert(bgfx::isValid(nonMorphAffectedVBHandle));
	m_nonMorphAffectedVBHandle = nonMorphAffectedVBHandle.idx;
	
	//2. Final Morph Affected
	cd::VertexFormat finalMorphAffectedVF;
	finalMorphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	finalMorphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::MorphWeght, cd::AttributeValueType::Float, 1U);
	
	bgfx::VertexLayout finalMorphAffectedVL;
	VertexLayoutUtility::CreateVertexLayout(finalMorphAffectedVL, finalMorphAffectedVF.GetVertexLayout());
	bgfx::DynamicVertexBufferHandle finalMorphAffectedVBHandle = bgfx::createDynamicVertexBuffer(meshVertexCount, finalMorphAffectedVL, BGFX_BUFFER_COMPUTE_READ_WRITE);
	assert(bgfx::isValid(finalMorphAffectedVBHandle));
	m_finalMorphAffectedVBHandle = finalMorphAffectedVBHandle.idx;

	//3. Active Morph Offest(Index) Length(Index) / Weight(Vertex)
	m_activeMorphOffestLengthIB.resize(morphCount * sizeof(uint32_t) * 2);
	auto activeMorphOffestLengthIBDataPtr = m_activeMorphOffestLengthIB.data();
	uint32_t activeMorphOffestLengthIBDataSize = 0U;

	cd::VertexFormat activeMorphWeightVF;
	activeMorphWeightVF.AddAttributeLayout(cd::VertexAttributeType::MorphWeght, cd::AttributeValueType::Float, 1U);
	const uint32_t activeMorphWeightVFStride = activeMorphWeightVF.GetStride();
	m_activeMorphWeightVB.resize(morphCount * (activeMorphWeightVFStride + 3 * placeholderSize));
	auto activeMorphWeightVBDataPtr = m_activeMorphWeightVB.data();
	uint32_t activeMorphWeightVBDataSize = 0U;

	uint32_t offset = 0U;
	for (uint32_t morphIndex = 0; morphIndex < morphCount; ++morphIndex)
	{
		uint32_t vertexCount = (*m_pMorphs)[morphIndex].GetVertexCount();
		uint32_t length = vertexCount;
		m_morphVertexCountSum += vertexCount;
		if (m_weights[morphIndex] > 0)
		{	
			m_activeMorphCount++;
			std::memcpy(&activeMorphOffestLengthIBDataPtr[activeMorphOffestLengthIBDataSize], &offset, sizeof(offset));
			activeMorphOffestLengthIBDataSize += sizeof(offset);
			std::memcpy(&activeMorphOffestLengthIBDataPtr[activeMorphOffestLengthIBDataSize], &length, sizeof(length));
			activeMorphOffestLengthIBDataSize += sizeof(length);

			std::memcpy(&activeMorphWeightVBDataPtr[activeMorphWeightVBDataSize], &m_weights[morphIndex], sizeof(float));
			activeMorphWeightVBDataSize += sizeof(float);
			std::memcpy(&activeMorphWeightVBDataPtr[activeMorphWeightVBDataSize], &placeholder, placeholderSize);
			activeMorphWeightVBDataSize += placeholderSize;
			std::memcpy(&activeMorphWeightVBDataPtr[activeMorphWeightVBDataSize], &placeholder, placeholderSize);
			activeMorphWeightVBDataSize += placeholderSize;
			std::memcpy(&activeMorphWeightVBDataPtr[activeMorphWeightVBDataSize], &placeholder, placeholderSize);
			activeMorphWeightVBDataSize += placeholderSize;
		}
		offset += length;
	}

	const bgfx::Memory* pActiveMorphOffestLengthIBRef = bgfx::makeRef(m_activeMorphOffestLengthIB.data(), static_cast<uint32_t>(m_activeMorphOffestLengthIB.size()));
	bgfx::DynamicIndexBufferHandle activeMorphOffestLengthIBHandle = bgfx::createDynamicIndexBuffer(pActiveMorphOffestLengthIBRef, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_INDEX32);
	assert(bgfx::isValid(activeMorphOffestLengthIBHandle));
	m_activeMorphOffestLengthIBHandle = activeMorphOffestLengthIBHandle.idx;

	bgfx::VertexLayout activeMorphWeightVL;
	VertexLayoutUtility::CreateVertexLayout(activeMorphWeightVL, activeMorphWeightVF.GetVertexLayout());
	const bgfx::Memory* pActiveMorphWeightVBRef = bgfx::makeRef(m_activeMorphWeightVB.data(), static_cast<uint32_t>(m_activeMorphWeightVB.size()));
	bgfx::DynamicVertexBufferHandle activeMorphWeightVBHandle = bgfx::createDynamicVertexBuffer(pActiveMorphWeightVBRef, activeMorphWeightVL, BGFX_BUFFER_COMPUTE_READ);
	assert(bgfx::isValid(activeMorphWeightVBHandle));
	m_activeMorphWeightVBHandle = activeMorphWeightVBHandle.idx;

	//4. Blend Shape Vertex Buffer
	m_allMorphVertexIDIB.resize(m_morphVertexCountSum * sizeof(uint32_t));
	auto allMorphVertexIDDataPtr = m_allMorphVertexIDIB.data();
	auto allMorphVertexIDDataSize = 0U;

	cd::VertexFormat allMorphVertexPosVF;
	allMorphVertexPosVF.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	const uint32_t allMorphVertexPosVFStride = allMorphVertexPosVF.GetStride();
	m_allMorphVertexPosVB.resize(m_morphVertexCountSum * (allMorphVertexPosVFStride + placeholderSize));
	auto allMorphVertexPosDataPtr = m_allMorphVertexPosVB.data();
	auto allMorphVertexPosDataSize = 0U;

	for (uint32_t morphIndex = 0; morphIndex < morphCount; ++morphIndex)
	{
		uint32_t morphVertexCount = (*m_pMorphs)[morphIndex].GetVertexCount();
		for (uint32_t vertexIndex = 0U; vertexIndex < morphVertexCount; vertexIndex++)
		{
			uint32_t vertexIDData= (*m_pMorphs)[morphIndex].GetVertexSourceID(vertexIndex).Data();
			std::memcpy(&allMorphVertexIDDataPtr[allMorphVertexIDDataSize], &vertexIDData, sizeof(vertexIDData));
			allMorphVertexIDDataSize += sizeof(vertexIDData);

			std::memcpy(&allMorphVertexPosDataPtr[allMorphVertexPosDataSize], (*m_pMorphs)[morphIndex].GetVertexPosition(vertexIndex).Begin(), positionSize);
			allMorphVertexPosDataSize += positionSize;
			std::memcpy(&allMorphVertexPosDataPtr[allMorphVertexPosDataSize], &placeholder, placeholderSize);
			allMorphVertexPosDataSize += placeholderSize;
		}
	}
	const bgfx::Memory* pAllMorphVertexIDIBRef = bgfx::makeRef(m_allMorphVertexIDIB.data(), static_cast<uint32_t>(m_allMorphVertexIDIB.size()));
	bgfx::IndexBufferHandle allMorphVertexIDIBHandle = bgfx::createIndexBuffer(pAllMorphVertexIDIBRef, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_INDEX32);
	assert(bgfx::isValid(allMorphVertexIDIBHandle));
	m_allMorphVertexIDIBHandle = allMorphVertexIDIBHandle.idx;

	bgfx::VertexLayout allMorphVertexPosVL;
	VertexLayoutUtility::CreateVertexLayout(allMorphVertexPosVL, allMorphVertexPosVF.GetVertexLayout());
	const bgfx::Memory* pAllMorphVertexPosVBRef = bgfx::makeRef(m_allMorphVertexPosVB.data(), static_cast<uint32_t>(m_allMorphVertexPosVB.size()));

	bgfx::VertexBufferHandle allMorphVertexPosVBHandle = bgfx::createVertexBuffer(pAllMorphVertexPosVBRef, allMorphVertexPosVL, BGFX_BUFFER_COMPUTE_READ);
	assert(bgfx::isValid(allMorphVertexPosVBHandle));
	m_allMorphVertexPosVBHandle = allMorphVertexPosVBHandle.idx;
	
	SetDirty(true);
}

void BlendShapeComponent::Update()
{
	const uint32_t meshVertexCount = m_pMesh->GetVertexCount();
	const uint32_t morphCount = static_cast<uint32_t>(m_pMorphs->size());
	m_vertexCount = meshVertexCount;
	float placeholder = 0.0f;
	uint32_t placeholderSize = sizeof(placeholder);

	//m_activeMorphOffestLengthIB.resize(morphCount * sizeof(uint32_t) * 2);
	auto activeMorphOffestLengthIBDataPtr = m_activeMorphOffestLengthIB.data();
	uint32_t activeMorphOffestLengthIBDataSize = 0U;

	//m_activeMorphWeightVB.resize(morphCount * (activeMorphWeightVFStride + 3 * placeholderSize));
	auto activeMorphWeightVBDataPtr = m_activeMorphWeightVB.data();
	uint32_t activeMorphWeightVBDataSize = 0U;

	uint32_t offset = 0U;
	m_morphVertexCountSum = 0U;
	for (uint32_t morphIndex = 0; morphIndex < morphCount; ++morphIndex)
	{
		uint32_t vertexCount = (*m_pMorphs)[morphIndex].GetVertexCount();
		uint32_t length = vertexCount;
		m_morphVertexCountSum += vertexCount;
		if (m_weights[morphIndex] > 0)
		{
			m_activeMorphCount++;
			std::memcpy(&activeMorphOffestLengthIBDataPtr[activeMorphOffestLengthIBDataSize], &offset, sizeof(offset));
			activeMorphOffestLengthIBDataSize += sizeof(offset);
			std::memcpy(&activeMorphOffestLengthIBDataPtr[activeMorphOffestLengthIBDataSize], &length, sizeof(length));
			activeMorphOffestLengthIBDataSize += sizeof(length);

			std::memcpy(&activeMorphWeightVBDataPtr[activeMorphWeightVBDataSize], &m_weights[morphIndex], sizeof(float));
			activeMorphWeightVBDataSize += sizeof(float);
			std::memcpy(&activeMorphWeightVBDataPtr[activeMorphWeightVBDataSize], &placeholder, placeholderSize);
			activeMorphWeightVBDataSize += placeholderSize;
			std::memcpy(&activeMorphWeightVBDataPtr[activeMorphWeightVBDataSize], &placeholder, placeholderSize);
			activeMorphWeightVBDataSize += placeholderSize;
			std::memcpy(&activeMorphWeightVBDataPtr[activeMorphWeightVBDataSize], &placeholder, placeholderSize);
			activeMorphWeightVBDataSize += placeholderSize;
		}
		offset += length;
	}

	const bgfx::Memory* pblendShapeDynamicBuffer1Ref = bgfx::makeRef(m_activeMorphOffestLengthIB.data(), static_cast<uint32_t>(m_activeMorphOffestLengthIB.size()));
	bgfx::update(bgfx::DynamicIndexBufferHandle{m_activeMorphOffestLengthIBHandle},0, pblendShapeDynamicBuffer1Ref);

	const bgfx::Memory* pblendShapeDynamicBuffer2Ref = bgfx::makeRef(m_activeMorphWeightVB.data(), static_cast<uint32_t>(m_activeMorphWeightVB.size()));
	bgfx::update(bgfx::DynamicVertexBufferHandle{m_activeMorphWeightVBHandle}, 0, pblendShapeDynamicBuffer2Ref);

}

}