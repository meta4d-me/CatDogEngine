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
	cd::VertexFormat morphVertexFormat;
	morphVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	morphVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	morphVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	morphVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);

	const uint32_t meshVertexCount = m_pMesh->GetVertexCount();
	const uint32_t vertexFormatStride = morphVertexFormat.GetStride();
	m_dynamicVertexBuffer.resize(meshVertexCount * vertexFormatStride);
	m_dynamicVertexBufferData.resize(meshVertexCount * vertexFormatStride / 4, 0);

	auto currentDataPtr = m_dynamicVertexBuffer.data();

	float weightSum = 0.0f;
	std::vector<cd::Point> positions;
	std::vector<cd::Direction> normals;
	std::vector<cd::Direction> tangents;
	std::vector<float> weightSums;
	positions.resize(meshVertexCount, cd::Point{ 0, 0, 0 });
	normals.resize(meshVertexCount, cd::Direction{ 0, 0, 0 });
	tangents.resize(meshVertexCount, cd::Direction{ 0, 0, 0 });
	weightSums.resize(meshVertexCount, 0);

	/*for (auto& morph : *m_pMorphs)// (*m_pMorphs)[index]
	{
		float weight = morph.GetWeight();
		weight = 0.6f;
		uint32_t morphVertexCount = morph.GetVertexCount();
		for (uint32_t i = 0U; i < morphVertexCount; i++)
		{
			cd::VertexID vertexID = morph.GetVertexSourceID(i);
			positions[vertexID.Data()] += morph.GetVertexPosition(i) * weight;
			normals[vertexID.Data()]  += morph.GetVertexNormal(i)  * weight;
			tangents[vertexID.Data()] += morph.GetVertexTangent(i) * weight;
			weightSums[vertexID.Data()] += weight;
		}
	}*/
	for (uint32_t j = 0U; j < m_pMorphs->size(); j++)// 
	{
		float weight = (*m_pMorphs)[j].GetWeight();
		weight = 5.0f;
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
	uint16_t stride = 11;
	for (uint32_t i = 0U; i < meshVertexCount; i++)
	{
		float sourceMeshWeight = 1.0f - weightSums[i];
		positions[i] += m_pMesh->GetVertexPosition(i) * sourceMeshWeight;
		normals[i] += m_pMesh->GetVertexNormal(i);// *sourceMeshWeight;
		tangents[i] += m_pMesh->GetVertexTangent(i);// *sourceMeshWeight;
		normals[i].Normalize();
		tangents[i].Normalize();

		m_dynamicVertexBufferData[i * stride + 0] = positions[i].x();
		m_dynamicVertexBufferData[i * stride + 1] = positions[i].y();
		m_dynamicVertexBufferData[i * stride + 2] = positions[i].z();
		m_dynamicVertexBufferData[i * stride + 3] = normals[i].x();
		m_dynamicVertexBufferData[i * stride + 4] = normals[i].y();
		m_dynamicVertexBufferData[i * stride + 5] = normals[i].z();
		m_dynamicVertexBufferData[i * stride + 6] = tangents[i].x();
		m_dynamicVertexBufferData[i * stride + 7] = tangents[i].y();
		m_dynamicVertexBufferData[i * stride + 8] = tangents[i].z();
		m_dynamicVertexBufferData[i * stride + 9] = m_pMesh->GetVertexUV(0, i).x();
		m_dynamicVertexBufferData[i * stride + 10] = m_pMesh->GetVertexUV(0, i).y();
	}
	
	std::memcpy(currentDataPtr, m_dynamicVertexBufferData.data(), m_dynamicVertexBufferData.size() * 4);
	
	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, morphVertexFormat.GetVertexLayout());
	const bgfx::Memory* pDynamicVertexBufferRef = bgfx::makeRef(m_dynamicVertexBuffer.data(), static_cast<uint32_t>(m_dynamicVertexBuffer.size()));

	bgfx::DynamicVertexBufferHandle dynamicVertexBufferHandle = bgfx::createDynamicVertexBuffer(pDynamicVertexBufferRef, vertexLayout);
	assert(bgfx::isValid(dynamicVertexBufferHandle));
	m_dynamicVertexBufferHandle = dynamicVertexBufferHandle.idx;

}

void BlendShapeComponent::Update()
{

}

}