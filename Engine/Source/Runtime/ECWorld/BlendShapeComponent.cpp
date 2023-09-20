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
	const uint32_t meshVertexCount = m_pMesh->GetVertexCount();
	m_vertexCount = meshVertexCount;
	const uint32_t morphCount = static_cast<uint32_t>(m_pMorphs->size());
	m_weights.resize(m_pMorphs->size(), 0.4f);

	cd::VertexFormat dynamicVertexFormat;
	dynamicVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	const uint32_t dynamicVertexFormatStride = dynamicVertexFormat.GetStride();
	
	cd::VertexFormat staticVertexFormat;
	staticVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	staticVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	staticVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	const uint32_t staticVertexFormatStride = staticVertexFormat.GetStride();

	m_vertexDynamicBuffer.resize(meshVertexCount * 16);
	m_vertexStaticBuffer2.resize(meshVertexCount * staticVertexFormatStride);
	
	auto dynamicDataPtr = m_vertexDynamicBuffer.data();
	uint32_t dynamicCurrentDataSize = 0U;
	auto staticCurrentDataPtr = m_vertexStaticBuffer2.data();
	uint32_t staticCurrentDataSize = 0U;

	uint32_t dataPosSize = cd::Point::Size * sizeof(cd::Point::ValueType);
	uint32_t dataNorSize = cd::Direction::Size * sizeof(cd::Direction::ValueType);
	uint32_t dataTanSize = cd::Direction::Size * sizeof(cd::Direction::ValueType);
	uint32_t dataUVSize = cd::UV::Size * sizeof(cd::UV::ValueType);
	float tmp = 0.0f;
	for (uint32_t vertexIndex = 0; vertexIndex < meshVertexCount; ++vertexIndex)
	{
		std::memcpy(&dynamicDataPtr[dynamicCurrentDataSize], m_pMesh->GetVertexPosition(vertexIndex).Begin(), dataPosSize);
		dynamicCurrentDataSize += dataPosSize;
		std::memcpy(&dynamicDataPtr[dynamicCurrentDataSize], &tmp, 4);
		dynamicCurrentDataSize += 4;
		std::memcpy(&staticCurrentDataPtr[staticCurrentDataSize], m_pMesh->GetVertexNormal(vertexIndex).Begin(), dataNorSize);
		staticCurrentDataSize += dataNorSize;
		std::memcpy(&staticCurrentDataPtr[staticCurrentDataSize], m_pMesh->GetVertexTangent(vertexIndex).Begin(), dataTanSize);
		staticCurrentDataSize += dataTanSize;
		std::memcpy(&staticCurrentDataPtr[staticCurrentDataSize], m_pMesh->GetVertexUV(0, vertexIndex).Begin(), dataUVSize);
		staticCurrentDataSize += dataUVSize;
	}

	bgfx::VertexLayout dynamicVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(dynamicVertexLayout, dynamicVertexFormat.GetVertexLayout());
	const bgfx::Memory* pDynamicVertexBufferRef = bgfx::makeRef(m_vertexDynamicBuffer.data(), static_cast<uint32_t>(m_vertexDynamicBuffer.size()));
	//bgfx::DynamicVertexBufferHandle dynamicVertexBufferHandle = bgfx::createDynamicVertexBuffer(bgfx::copy(m_vertexDynamicBuffer.data(), static_cast<uint32_t>(m_vertexDynamicBuffer.size())), dynamicVertexLayout, BGFX_BUFFER_COMPUTE_READ_WRITE);
	bgfx::VertexBufferHandle staticVertexBufferHandle1 = bgfx::createVertexBuffer(pDynamicVertexBufferRef, dynamicVertexLayout, BGFX_BUFFER_COMPUTE_READ);
	assert(bgfx::isValid(staticVertexBufferHandle1));
	m_vertexStaticBufferHandle1 = staticVertexBufferHandle1.idx;



	bgfx::VertexLayout staticVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(staticVertexLayout, staticVertexFormat.GetVertexLayout());
	const bgfx::Memory* pStaticVertexBufferRef = bgfx::makeRef(m_vertexStaticBuffer2.data(), static_cast<uint32_t>(m_vertexStaticBuffer2.size()));
	bgfx::VertexBufferHandle staticVertexBufferHandle = bgfx::createVertexBuffer(pStaticVertexBufferRef, staticVertexLayout);
	assert(bgfx::isValid(staticVertexBufferHandle));
	m_vertexStaticBufferHandle2 = staticVertexBufferHandle.idx;

	//3. Blend Shape Dynamic Vertex Buffer
	cd::VertexFormat blendShapeDynamicVertexFormat;
	blendShapeDynamicVertexFormat.AddAttributeLayout(cd::VertexAttributeType::MorphWeght, cd::AttributeValueType::Float, 1U);
	const uint32_t blendShapeVertexFormatStride = blendShapeDynamicVertexFormat.GetStride();

	m_blendShapeDynamicBuffer1.resize(morphCount * sizeof(uint32_t) * 2);
	m_blendShapeDynamicBuffer2.resize(morphCount * blendShapeVertexFormatStride * 4);

	auto bsdb1DataPtr = m_blendShapeDynamicBuffer1.data();
	auto bsdb2DataPtr = m_blendShapeDynamicBuffer2.data();
	std::vector<float> sourceWeightData;
	sourceWeightData.resize(meshVertexCount * 1, 1.0f);
	uint32_t bsdb1DataSize = 0U;
	uint32_t bsdb2DataSize = 0U;
	uint32_t swdbDataSize = 0U;
	uint32_t offset = 0U;
	uint32_t morphVertexCountSum = 0U;
	for (uint32_t morphIndex = 0; morphIndex < morphCount; ++morphIndex)
	{
		uint32_t vertexCount = (*m_pMorphs)[morphIndex].GetVertexCount();
		uint32_t length = vertexCount;
		morphVertexCountSum += vertexCount;
		if (m_weights[morphIndex] > 0)
		{	
			m_activeMorphCount++;
			std::memcpy(&bsdb1DataPtr[bsdb1DataSize], &offset, sizeof(offset));
			bsdb1DataSize += sizeof(offset);
			std::memcpy(&bsdb1DataPtr[bsdb1DataSize], &length, sizeof(length));
			bsdb1DataSize += sizeof(length);
			std::memcpy(&bsdb2DataPtr[bsdb2DataSize], &m_weights[morphIndex], sizeof(float));
			bsdb2DataSize += sizeof(float);
			std::memcpy(&bsdb2DataPtr[bsdb2DataSize], &m_weights[morphIndex], sizeof(float));
			bsdb2DataSize += sizeof(float);
			std::memcpy(&bsdb2DataPtr[bsdb2DataSize], &m_weights[morphIndex], sizeof(float));
			bsdb2DataSize += sizeof(float);
			std::memcpy(&bsdb2DataPtr[bsdb2DataSize], &m_weights[morphIndex], sizeof(float));
			bsdb2DataSize += sizeof(float);
		}
		offset += length;
	}

	const bgfx::Memory* pblendShapeDynamicBuffer1Ref = bgfx::makeRef(m_blendShapeDynamicBuffer1.data(), static_cast<uint32_t>(m_blendShapeDynamicBuffer1.size()));
	bgfx::DynamicIndexBufferHandle blendShapeDynamicIndexBufferHandle = bgfx::createDynamicIndexBuffer(pblendShapeDynamicBuffer1Ref, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_INDEX32);

	assert(bgfx::isValid(blendShapeDynamicIndexBufferHandle));
	m_blendShapeDynamicBufferHandle1 = blendShapeDynamicIndexBufferHandle.idx;

	bgfx::VertexLayout blendShapeDynamicVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(blendShapeDynamicVertexLayout, blendShapeDynamicVertexFormat.GetVertexLayout());
	const bgfx::Memory* pblendShapeDynamicBuffer2Ref = bgfx::makeRef(m_blendShapeDynamicBuffer2.data(), static_cast<uint32_t>(m_blendShapeDynamicBuffer2.size()));
	bgfx::DynamicVertexBufferHandle blendShapeDynamicVertexBufferHandle = bgfx::createDynamicVertexBuffer(pblendShapeDynamicBuffer2Ref, blendShapeDynamicVertexLayout, BGFX_BUFFER_COMPUTE_READ);
	assert(bgfx::isValid(blendShapeDynamicVertexBufferHandle));
	m_blendShapeDynamicBufferHandle2 = blendShapeDynamicVertexBufferHandle.idx;

	//4. Blend Shape Vertex Buffer
	cd::VertexFormat blendShapeStaticVertexFormat;
	blendShapeStaticVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	const uint32_t blendShapeStaticVertexFormatStride = blendShapeStaticVertexFormat.GetStride();

	m_blendShapeStaticBuffer1.resize(morphVertexCountSum * sizeof(uint32_t));
	m_blendShapeStaticBuffer2.resize(morphVertexCountSum * 16);

	auto bssb1DataPtr = m_blendShapeStaticBuffer1.data();
	auto bssb1DataSize = 0U;
	auto bssb2DataPtr = m_blendShapeStaticBuffer2.data();
	auto bssb2DataSize = 0U;

	for (uint32_t morphIndex = 0; morphIndex < morphCount; ++morphIndex)
	{
		uint32_t morphVertexCount = (*m_pMorphs)[morphIndex].GetVertexCount();
		for (uint32_t vertexIndex = 0U; vertexIndex < morphVertexCount; vertexIndex++)
		{
			uint32_t vertexIDData= (*m_pMorphs)[morphIndex].GetVertexSourceID(vertexIndex).Data();
			std::memcpy(&bssb1DataPtr[bssb1DataSize], &vertexIDData, sizeof(vertexIDData));
			bssb1DataSize += sizeof(vertexIDData);
			std::memcpy(&bssb2DataPtr[bssb2DataSize], (*m_pMorphs)[morphIndex].GetVertexPosition(vertexIndex).Begin(), dataPosSize);
			bssb2DataSize += dataPosSize;
			std::memcpy(&bssb2DataPtr[bssb2DataSize], &tmp, 4);
			bssb2DataSize += 4;
		}
	}
	const bgfx::Memory* pBlendShapeStaticIndexBufferRef = bgfx::makeRef(m_blendShapeStaticBuffer1.data(), static_cast<uint32_t>(m_blendShapeStaticBuffer1.size()));
	bgfx::IndexBufferHandle blendShapeStaticIndexBufferHandle = bgfx::createIndexBuffer(pBlendShapeStaticIndexBufferRef, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_INDEX32);
	assert(bgfx::isValid(blendShapeStaticIndexBufferHandle));
	m_blendShapeStaticBufferHandle1 = blendShapeStaticIndexBufferHandle.idx;

	bgfx::VertexLayout blendShapeStaticVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(blendShapeStaticVertexLayout, blendShapeStaticVertexFormat.GetVertexLayout());
	const bgfx::Memory* pBlendShapeStaticVertexBufferRef = bgfx::makeRef(m_blendShapeStaticBuffer2.data(), static_cast<uint32_t>(m_blendShapeStaticBuffer2.size()));

	bgfx::VertexBufferHandle blendShapeStaticVertexBufferHandle = bgfx::createVertexBuffer(pBlendShapeStaticVertexBufferRef, blendShapeStaticVertexLayout, BGFX_BUFFER_COMPUTE_READ);
	assert(bgfx::isValid(blendShapeStaticVertexBufferHandle));
	m_blendShapeStaticBufferHandle2 = blendShapeStaticVertexBufferHandle.idx;
	
	//


	cd::VertexFormat dynamicPosWeightFormat;
	dynamicPosWeightFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	dynamicPosWeightFormat.AddAttributeLayout(cd::VertexAttributeType::MorphWeght, cd::AttributeValueType::Float, 1U);
	bgfx::VertexLayout dynamicPosWeightVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(dynamicPosWeightVertexLayout, dynamicPosWeightFormat.GetVertexLayout());

	bgfx::DynamicVertexBufferHandle dynamicVertexBufferHandle = bgfx::createDynamicVertexBuffer(meshVertexCount, dynamicPosWeightVertexLayout, BGFX_BUFFER_COMPUTE_READ_WRITE);
	assert(bgfx::isValid(dynamicVertexBufferHandle));
	m_vertexDynamicBufferHandle = dynamicVertexBufferHandle.idx;

	SetDirty(true);
}

void BlendShapeComponent::Update()
{
	const uint32_t meshVertexCount = m_pMesh->GetVertexCount();
	const uint32_t morphCount = static_cast<uint32_t>(m_pMorphs->size());
	m_vertexCount = meshVertexCount;

	cd::VertexFormat blendShapeDynamicVertexFormat;
	blendShapeDynamicVertexFormat.AddAttributeLayout(cd::VertexAttributeType::MorphWeght, cd::AttributeValueType::Float, 1U);
	const uint32_t blendShapeVertexFormatStride = blendShapeDynamicVertexFormat.GetStride();
	m_blendShapeDynamicBuffer1.resize(morphCount * sizeof(uint32_t) * 2);
	m_blendShapeDynamicBuffer2.resize(morphCount * blendShapeVertexFormatStride * 4);

	auto bsdb1DataPtr = m_blendShapeDynamicBuffer1.data();
	auto bsdb2DataPtr = m_blendShapeDynamicBuffer2.data();
	uint32_t bsdb1DataSize = 0U;
	uint32_t bsdb2DataSize = 0U;
	uint32_t offset = 0U;
	uint32_t morphVertexCountSum = 0U;
	m_activeMorphCount = 0U;
	std::vector<float> lengths;
	for (uint32_t morphIndex = 0; morphIndex < morphCount; ++morphIndex)
	{
		uint32_t vertexCount = (*m_pMorphs)[morphIndex].GetVertexCount();
		uint32_t length = vertexCount;
		morphVertexCountSum += vertexCount;
		if (m_weights[morphIndex] > 0)
		{
			m_activeMorphCount++;
			std::memcpy(&bsdb1DataPtr[bsdb1DataSize], &offset, sizeof(offset));
			bsdb1DataSize += sizeof(offset);
			std::memcpy(&bsdb1DataPtr[bsdb1DataSize], &length, sizeof(length));
			bsdb1DataSize += sizeof(length);
			std::memcpy(&bsdb2DataPtr[bsdb2DataSize], &m_weights[morphIndex], sizeof(float));
			bsdb2DataSize += sizeof(float);
			std::memcpy(&bsdb2DataPtr[bsdb2DataSize], &m_weights[morphIndex], sizeof(float));
			bsdb2DataSize += sizeof(float);
			std::memcpy(&bsdb2DataPtr[bsdb2DataSize], &m_weights[morphIndex], sizeof(float));
			bsdb2DataSize += sizeof(float);
			std::memcpy(&bsdb2DataPtr[bsdb2DataSize], &m_weights[morphIndex], sizeof(float));
			bsdb2DataSize += sizeof(float);
		}
		offset += length;
	}

	const bgfx::Memory* pblendShapeDynamicBuffer1Ref = bgfx::makeRef(m_blendShapeDynamicBuffer1.data(), static_cast<uint32_t>(m_blendShapeDynamicBuffer1.size()));
	bgfx::update(bgfx::DynamicIndexBufferHandle{m_blendShapeDynamicBufferHandle1},0, pblendShapeDynamicBuffer1Ref);

	const bgfx::Memory* pblendShapeDynamicBuffer2Ref = bgfx::makeRef(m_blendShapeDynamicBuffer2.data(), static_cast<uint32_t>(m_blendShapeDynamicBuffer2.size()));
	bgfx::update(bgfx::DynamicVertexBufferHandle{m_blendShapeDynamicBufferHandle2}, 0, pblendShapeDynamicBuffer2Ref);

}

}