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

	const uint32_t vertexCount = m_pMesh->GetVertexCount();
	const uint32_t vertexFormatStride = morphVertexFormat.GetStride();
	m_dynamicVertexBuffer.resize(vertexCount * vertexFormatStride);
	m_dynamicVertexBufferData.resize(vertexCount * vertexFormatStride / 2, 0);

	auto currentDataPtr = m_dynamicVertexBuffer.data();

	float weightSum = 0;
	std::vector<cd::Point> positions;
	std::vector<cd::Direction> normals;
	std::vector<cd::Direction> tangents;
	positions.resize(vertexCount, cd::Point{ 0, 0, 0 });
	normals.resize(vertexCount, cd::Direction{ 0, 0, 0 });
	tangents.resize(vertexCount, cd::Direction{ 0, 0, 0 });

	for(auto & morph : *m_pMorphs)
	{
		float weight = morph.GetWeight();
		weightSum += weight;
		uint32_t vertexCount = morph.GetVertexCount();
		for (uint32_t i = 0U; i < vertexCount; i++)
		{
			cd::VertexID vertexID = morph.GetVertexSourceID(i);
			cd::Point position = morph.GetVertexPosition(i);
			//positions[vertexID.Data()] += morph.GetVertexPosition(i) * weight;
			//normals[vertexID.Data()]  += morph.GetVertexNormal(i)  * weight;
			//tangents[vertexID.Data()] += morph.GetVertexTangent(i) * weight
			int a =0;
		}
	}
	m_sourceMeshWeight = 1 - weightSum;

	for (uint32_t i = 0U; i < vertexCount; i++)
	{
		normals[i].Normalize();
		tangents[i].Normalize();
		m_dynamicVertexBufferData[i * 9 + 0] += positions[i].x();
		m_dynamicVertexBufferData[i * 9 + 1] += positions[i].y();
		m_dynamicVertexBufferData[i * 9 + 2] += positions[i].z();
		m_dynamicVertexBufferData[i * 9 + 3] += normals[i].x();
		m_dynamicVertexBufferData[i * 9 + 4] += normals[i].y();
		m_dynamicVertexBufferData[i * 9 + 5] += normals[i].z();
		m_dynamicVertexBufferData[i * 9 + 6] += tangents[i].x();
		m_dynamicVertexBufferData[i * 9 + 7] += tangents[i].y();
		m_dynamicVertexBufferData[i * 9 + 8] += tangents[i].z();
	}
	
	std::memcpy(currentDataPtr, &m_dynamicVertexBufferData, m_dynamicVertexBuffer.size());
	
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