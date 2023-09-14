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
	m_morphs.clear();
	m_dynamicVertexBufferHandle = UINT16_MAX;
}
*/
void BlendShapeComponent::Build()
{
	cd::VertexFormat morphDynamicVertexFormat;
	morphDynamicVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	morphDynamicVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	morphDynamicVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	const uint32_t vertexDynamicFormatStride = morphDynamicVertexFormat.GetStride();
	
	cd::VertexFormat morphStaticVertexFormat;
	morphStaticVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	const uint32_t vertexStaticFormatStride = morphDynamicVertexFormat.GetStride();

	const uint32_t meshVertexCount = m_pMesh->GetVertexCount();
	
	m_weights.resize(m_pMorphs->size(), 0.0f);
	m_dynamicVertexBuffer.resize(meshVertexCount * vertexDynamicFormatStride);
	m_dynamicVertexBufferData.resize(meshVertexCount * vertexDynamicFormatStride / 4, 0);
	m_staticVertexBuffer.resize(meshVertexCount * vertexStaticFormatStride);
	m_staticVertexBufferData.resize(meshVertexCount * vertexStaticFormatStride / 4, 0);

	auto currentDynamicDataPtr = m_dynamicVertexBuffer.data();
	auto currentStaticDataPtr = m_staticVertexBuffer.data();

	float weightSum = 0.0f;
	std::vector<cd::Point> positions;
	std::vector<cd::Direction> normals;
	std::vector<cd::Direction> tangents;
	std::vector<float> weightSums;
	positions.resize(meshVertexCount, cd::Point{ 0, 0, 0 });
	normals.resize(meshVertexCount, cd::Direction{ 0, 0, 0 });
	tangents.resize(meshVertexCount, cd::Direction{ 0, 0, 0 });
	weightSums.resize(meshVertexCount, 0);

	for (uint32_t j = 0U; j < m_pMorphs->size(); j++)// 
	{
		//float weight = (*m_pMorphs)[j].GetWeight();
		//weight = 0.1f;
		float weight = m_weights[j];
		uint32_t morphVertexCount = (*m_pMorphs)[j].GetVertexCount();
		for (uint32_t i = 0U; i < morphVertexCount; i++)
		{
			cd::VertexID vertexID = (*m_pMorphs)[j].GetVertexSourceID(i);
			positions[vertexID.Data()] += (*m_pMorphs)[j].GetVertexPosition(i) * weight;
			normals[vertexID.Data()] += (*m_pMorphs)[j].GetVertexNormal(i) * weight;
			tangents[vertexID.Data()] += (*m_pMorphs)[j].GetVertexTangent(i) * weight;
			weightSums[vertexID.Data()] += weight;
		}
	}
	uint16_t dynamicStride = 9;
	uint16_t staticStride = 2;
	for (uint32_t i = 0U; i < meshVertexCount; i++)
	{
		float sourceMeshWeight = 1.0f - weightSums[i];
		positions[i] += m_pMesh->GetVertexPosition(i) * sourceMeshWeight;
		normals[i] += m_pMesh->GetVertexNormal(i);// *sourceMeshWeight;
		tangents[i] += m_pMesh->GetVertexTangent(i);// *sourceMeshWeight;
		normals[i].Normalize();
		tangents[i].Normalize();

		m_dynamicVertexBufferData[i * dynamicStride + 0] = positions[i].x();
		m_dynamicVertexBufferData[i * dynamicStride + 1] = positions[i].y();
		m_dynamicVertexBufferData[i * dynamicStride + 2] = positions[i].z();
		m_dynamicVertexBufferData[i * dynamicStride + 3] = normals[i].x();
		m_dynamicVertexBufferData[i * dynamicStride + 4] = normals[i].y();
		m_dynamicVertexBufferData[i * dynamicStride + 5] = normals[i].z();
		m_dynamicVertexBufferData[i * dynamicStride + 6] = tangents[i].x();
		m_dynamicVertexBufferData[i * dynamicStride + 7] = tangents[i].y();
		m_dynamicVertexBufferData[i * dynamicStride + 8] = tangents[i].z();
		m_staticVertexBufferData[i * staticStride + 0] = m_pMesh->GetVertexUV(0, i).x();
		m_staticVertexBufferData[i * staticStride + 1] = m_pMesh->GetVertexUV(0, i).y();
	}
	
	std::memcpy(currentDynamicDataPtr, m_dynamicVertexBufferData.data(), m_dynamicVertexBufferData.size() * 4);
	std::memcpy(currentStaticDataPtr, m_staticVertexBufferData.data(), m_staticVertexBufferData.size() * 4);
	
	bgfx::VertexLayout dynamicVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(dynamicVertexLayout, morphDynamicVertexFormat.GetVertexLayout());
	const bgfx::Memory* pDynamicVertexBufferRef = bgfx::makeRef(m_dynamicVertexBuffer.data(), static_cast<uint32_t>(m_dynamicVertexBuffer.size()));

	bgfx::DynamicVertexBufferHandle dynamicVertexBufferHandle = bgfx::createDynamicVertexBuffer(pDynamicVertexBufferRef, dynamicVertexLayout);
	assert(bgfx::isValid(dynamicVertexBufferHandle));
	m_dynamicVertexBufferHandle = dynamicVertexBufferHandle.idx;

	bgfx::VertexLayout staticVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(staticVertexLayout, morphStaticVertexFormat.GetVertexLayout());
	const bgfx::Memory* pStaticVertexBufferRef = bgfx::makeRef(m_staticVertexBuffer.data(), static_cast<uint32_t>(m_staticVertexBuffer.size()));

	bgfx::VertexBufferHandle staticVertexBufferHandle = bgfx::createVertexBuffer(pStaticVertexBufferRef, staticVertexLayout);
	assert(bgfx::isValid(staticVertexBufferHandle));
	m_staticVertexBufferHandle = staticVertexBufferHandle.idx;

	SetIsDirty(false);
}

void BlendShapeComponent::Update()
{
	cd::VertexFormat morphDynamicVertexFormat;
	morphDynamicVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	morphDynamicVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	morphDynamicVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	const uint32_t vertexDynamicFormatStride = morphDynamicVertexFormat.GetStride();

	const uint32_t meshVertexCount = m_pMesh->GetVertexCount();
	//std::vector<float> m_dynamicVertexBufferData;
	//std::vector<float> m_staticVertexBufferData;
	std::vector<std::byte> dynamicVertexBuffer;
	dynamicVertexBuffer.resize(meshVertexCount * vertexDynamicFormatStride);
	auto currentDynamicDataPtr = m_dynamicVertexBuffer.data();

	float weightSum = 0.0f;
	std::vector<cd::Point> positions;
	std::vector<cd::Direction> normals;
	std::vector<cd::Direction> tangents;
	std::vector<float> weightSums;
	positions.resize(meshVertexCount, cd::Point{ 0, 0, 0 });
	normals.resize(meshVertexCount, cd::Direction{ 0, 0, 0 });
	tangents.resize(meshVertexCount, cd::Direction{ 0, 0, 0 });
	weightSums.resize(meshVertexCount, 0);

	for (uint32_t j = 0U; j < m_pMorphs->size(); j++)// 
	{
		//float weight = (*m_pMorphs)[j].GetWeight();
		//weight = 0.1f;
		float weight = m_weights[j];
		uint32_t morphVertexCount = (*m_pMorphs)[j].GetVertexCount();
		for (uint32_t i = 0U; i < morphVertexCount; i++)
		{
			cd::VertexID vertexID = (*m_pMorphs)[j].GetVertexSourceID(i);
			cd::Point pos = (*m_pMorphs)[j].GetVertexPosition(i);
			cd::Point posm = m_pMesh->GetVertexPosition(vertexID.Data());
			cd::Direction norm = (*m_pMorphs)[j].GetVertexNormal(i);
			cd::Direction tan = (*m_pMorphs)[j].GetVertexTangent(i);
			positions[vertexID.Data()] += (*m_pMorphs)[j].GetVertexPosition(i) * weight;
			normals[vertexID.Data()] += (*m_pMorphs)[j].GetVertexNormal(i) * weight;
			tangents[vertexID.Data()] += (*m_pMorphs)[j].GetVertexTangent(i) * weight;
			weightSums[vertexID.Data()] += weight;
		}
	}
	uint16_t dynamicStride = 9;
	for (uint32_t i = 0U; i < meshVertexCount; i++)
	{
		float sourceMeshWeight = 1.0f - weightSums[i];
		positions[i] += m_pMesh->GetVertexPosition(i) * sourceMeshWeight;
		normals[i] += m_pMesh->GetVertexNormal(i);// *sourceMeshWeight;
		tangents[i] += m_pMesh->GetVertexTangent(i);// *sourceMeshWeight;
		normals[i].Normalize();
		tangents[i].Normalize();

		m_dynamicVertexBufferData[i * dynamicStride + 0] = positions[i].x();
		m_dynamicVertexBufferData[i * dynamicStride + 1] = positions[i].y();
		m_dynamicVertexBufferData[i * dynamicStride + 2] = positions[i].z();
		m_dynamicVertexBufferData[i * dynamicStride + 3] = normals[i].x();
		m_dynamicVertexBufferData[i * dynamicStride + 4] = normals[i].y();
		m_dynamicVertexBufferData[i * dynamicStride + 5] = normals[i].z();
		m_dynamicVertexBufferData[i * dynamicStride + 6] = tangents[i].x();
		m_dynamicVertexBufferData[i * dynamicStride + 7] = tangents[i].y();
		m_dynamicVertexBufferData[i * dynamicStride + 8] = tangents[i].z();
	}

	std::memcpy(currentDynamicDataPtr, m_dynamicVertexBufferData.data(), m_dynamicVertexBufferData.size() * 4);
	const bgfx::Memory* pDynamicVertexBufferRef = bgfx::makeRef(m_dynamicVertexBuffer.data(), static_cast<uint32_t>(m_dynamicVertexBuffer.size()));
	bgfx::update(bgfx::DynamicVertexBufferHandle{m_dynamicVertexBufferHandle}, 0U, pDynamicVertexBufferRef);

	SetIsDirty(false);
}

}