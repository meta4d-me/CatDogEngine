#include "MaterialType.h"

#include "Math/Vector.hpp"
#include "Scene/VertexFormat.h"

namespace engine
{

MaterialType MaterialType::GetPBRMaterialType()
{
	MaterialType pbr;
	pbr.m_materialName = "CDStandard";
	pbr.m_vertexShaderName = "vs_PBR.bin";
	pbr.m_fragmentShaderName = "fs_PBR.bin";

	cd::VertexFormat pbrVertexFormat;
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	pbr.SetVertexFormat(cd::MoveTemp(pbrVertexFormat));

	return pbr;
}

}