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
	m_weights.resize(m_morphCount, 0.2f);

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
	m_morphAffectedVB.resize(m_meshVertexCount * 16);
	auto morphAffectedVBDataPtr = m_morphAffectedVB.data();
	uint32_t morphAffectedVBDataSize = 0U;

	cd::VertexFormat nonMorphAffectedVF;//Morph Non-Affected Vertex Format
	nonMorphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	nonMorphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	nonMorphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	const uint32_t nonMorphAffectedVFStride = nonMorphAffectedVF.GetStride();
	m_nonMorphAffectedVB.resize(m_meshVertexCount * nonMorphAffectedVFStride);
	auto nonMorphAffectedDataPtr = m_nonMorphAffectedVB.data();
	uint32_t nonMorphAffectedVBDataSize = 0U;

	for (uint32_t vertexIndex = 0; vertexIndex < m_meshVertexCount; ++vertexIndex)
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
	finalMorphAffectedVF.AddAttributeLayout(cd::VertexAttributeType::BoneWeight, cd::AttributeValueType::Float, 1U);
	
	bgfx::VertexLayout finalMorphAffectedVL;
	VertexLayoutUtility::CreateVertexLayout(finalMorphAffectedVL, finalMorphAffectedVF.GetVertexLayout());
	bgfx::DynamicVertexBufferHandle finalMorphAffectedVBHandle = bgfx::createDynamicVertexBuffer(m_meshVertexCount, finalMorphAffectedVL, BGFX_BUFFER_COMPUTE_READ_WRITE);
	assert(bgfx::isValid(finalMorphAffectedVBHandle));
	m_finalMorphAffectedVBHandle = finalMorphAffectedVBHandle.idx;

	//3. Active Morph Offest(Index)+Length(Index)+Weight(Index)
	m_activeMorphOffestLengthWeightIB.resize(m_morphCount * sizeof(uint32_t) * 3);
	auto activeMorphOffestLengthIBDataPtr = m_activeMorphOffestLengthWeightIB.data();
	uint32_t activeMorphOffestLengthIBDataSize = 0U;

	uint32_t offset = 0U;
	for (uint32_t morphIndex = 0; morphIndex < m_morphCount; ++morphIndex)
	{
		uint32_t vertexCount = m_pMorphsData[morphIndex].GetVertexCount();
		uint32_t length = vertexCount;
		m_morphVertexCountSum += vertexCount;
		if (m_weights[morphIndex] > 0)
		{	
			m_activeMorphCount++;
			std::memcpy(&activeMorphOffestLengthIBDataPtr[activeMorphOffestLengthIBDataSize], &offset, sizeof(offset));
			activeMorphOffestLengthIBDataSize += sizeof(offset);
			std::memcpy(&activeMorphOffestLengthIBDataPtr[activeMorphOffestLengthIBDataSize], &length, sizeof(length));
			activeMorphOffestLengthIBDataSize += sizeof(length);
			std::memcpy(&activeMorphOffestLengthIBDataPtr[activeMorphOffestLengthIBDataSize], &m_weights[morphIndex], sizeof(float));
			activeMorphOffestLengthIBDataSize += sizeof(float);
		}
		offset += length;
	}

	const bgfx::Memory* pActiveMorphOffestLengthIBRef = bgfx::makeRef(m_activeMorphOffestLengthWeightIB.data(), static_cast<uint32_t>(m_activeMorphOffestLengthWeightIB.size()));
	bgfx::DynamicIndexBufferHandle activeMorphOffestLengthIBHandle = bgfx::createDynamicIndexBuffer(pActiveMorphOffestLengthIBRef, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_INDEX32);
	assert(bgfx::isValid(activeMorphOffestLengthIBHandle));
	m_activeMorphOffestLengthWeightIBHandle = activeMorphOffestLengthIBHandle.idx;

	//4. Blend Shape Vertex Buffer
	m_allMorphVertexIDPosIB.resize(m_morphVertexCountSum * (sizeof(uint32_t) + 3*sizeof(float)));
	auto allMorphVertexIDDataPtr = m_allMorphVertexIDPosIB.data();
	auto allMorphVertexIDDataSize = 0U;

	for (uint32_t morphIndex = 0; morphIndex < m_morphCount; ++morphIndex)
	{
		uint32_t morphVertexCount = m_pMorphsData[morphIndex].GetVertexCount();
		for (uint32_t vertexIndex = 0U; vertexIndex < morphVertexCount; vertexIndex++)
		{
			uint32_t vertexIDData= m_pMorphsData[morphIndex].GetVertexSourceID(vertexIndex).Data();
			std::memcpy(&allMorphVertexIDDataPtr[allMorphVertexIDDataSize], &vertexIDData, sizeof(vertexIDData));
			allMorphVertexIDDataSize += sizeof(vertexIDData);
			std::memcpy(&allMorphVertexIDDataPtr[allMorphVertexIDDataSize], m_pMorphsData[morphIndex].GetVertexPosition(vertexIndex).Begin(), positionSize);
			allMorphVertexIDDataSize += positionSize;
		}
	}
	const bgfx::Memory* pAllMorphVertexIDIBRef = bgfx::makeRef(m_allMorphVertexIDPosIB.data(), static_cast<uint32_t>(m_allMorphVertexIDPosIB.size()));
	bgfx::IndexBufferHandle allMorphVertexIDIBHandle = bgfx::createIndexBuffer(pAllMorphVertexIDIBRef, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_INDEX32);
	assert(bgfx::isValid(allMorphVertexIDIBHandle));
	m_allMorphVertexIDPosIBHandle = allMorphVertexIDIBHandle.idx;

	bgfx::DynamicIndexBufferHandle changedMorphIndexIBHandle = bgfx::createDynamicIndexBuffer(1+4*m_morphCount, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_INDEX32);
	assert(bgfx::isValid(changedMorphIndexIBHandle));
	m_changedMorphIndexIBHandle = changedMorphIndexIBHandle.idx;

	// 5. overall weight Vertex Buffer
	bgfx::DynamicVertexBufferHandle alphaVBHandle = bgfx::createDynamicVertexBuffer(m_meshVertexCount, finalMorphAffectedVL, BGFX_BUFFER_COMPUTE_READ_WRITE);
	assert(bgfx::isValid(alphaVBHandle));
	m_finalVBHandle = alphaVBHandle.idx;

	SetDirty(true);
}

void BlendShapeComponent::Update()
{


}

void BlendShapeComponent::UpdateChanged()
{
	uint32_t needUpdateCount = static_cast<uint32_t>(m_needUpdates.size());
	m_changedMorphIndexIB.resize((needUpdateCount * 4+1) * 4, std::byte{0});

	auto changedMorphIndexIBDataPtr = m_changedMorphIndexIB.data();
	uint32_t changedMorphIndexIBDataSize = 0U;
	
	std::memcpy(&changedMorphIndexIBDataPtr[changedMorphIndexIBDataSize], &needUpdateCount, sizeof(needUpdateCount));
	changedMorphIndexIBDataSize += sizeof(needUpdateCount);

	uint32_t morphIndex = 0;
	uint32_t offsetAndLength[2] = { 0U, 0U };
	float changedWeights[2] = {0.0f, 0.0f};// 0: before changed 1: after changed
	for (auto iter = m_needUpdates.begin(); iter != m_needUpdates.end(); )
	{
		uint32_t morphVertexCount = m_pMorphsData[morphIndex].GetVertexCount();
		if (morphIndex == iter->first)
		{
			offsetAndLength[1] = morphVertexCount;
			changedWeights[0] = iter->second;
			changedWeights[1] = m_weights[iter->first];
			std::memcpy(&changedMorphIndexIBDataPtr[changedMorphIndexIBDataSize], &offsetAndLength, sizeof(offsetAndLength));
			changedMorphIndexIBDataSize += sizeof(offsetAndLength);
			std::memcpy(&changedMorphIndexIBDataPtr[changedMorphIndexIBDataSize], &changedWeights, sizeof(changedWeights));
			changedMorphIndexIBDataSize += sizeof(changedWeights);
			iter++;
		}
		offsetAndLength[0] += morphVertexCount;
		morphIndex++;
	}

	const bgfx::Memory* pChangedMorphIndexIBRef = bgfx::makeRef(m_changedMorphIndexIB.data(), static_cast<uint32_t>(sizeof(m_changedMorphIndexIB)));
	bgfx::update(bgfx::DynamicIndexBufferHandle{m_changedMorphIndexIBHandle}, 0, pChangedMorphIndexIBRef);
}


}