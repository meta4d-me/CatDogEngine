#include "MaterialType.h"

#include "Math/Vector.hpp"

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
	pbr.SetRequiredVertexFormat(cd::MoveTemp(pbrVertexFormat));

	// Slot index should align to shader codes.
	// TODO : define them in a common C header file.
	pbr.AddRequiredTextureType(cd::MaterialTextureType::BaseColor, 0);
	pbr.AddRequiredTextureType(cd::MaterialTextureType::Normal, 1);
	pbr.AddRequiredTextureType(cd::MaterialTextureType::Metalness, 2);
	pbr.AddRequiredTextureType(cd::MaterialTextureType::Roughness, 2);

	pbr.AddOptionalTextureType(cd::MaterialTextureType::AO, 2);
	//pbr.AddOptionalTextureType(cd::MaterialTextureType::Emissive);

	return pbr;
}

void MaterialType::AddOptionalTextureType(cd::MaterialTextureType textureType, uint8_t slot)
{
	m_optionalTextureTypes.insert(textureType);
	m_textureTypeSlots[textureType] = slot;
}

void MaterialType::AddRequiredTextureType(cd::MaterialTextureType textureType, uint8_t slot)
{
	m_requiredTextureTypes.insert(textureType);
	m_textureTypeSlots[textureType] = slot;
}

std::optional<uint8_t> MaterialType::GetTextureSlot(cd::MaterialTextureType textureType)
{
	auto itTexture = m_textureTypeSlots.find(textureType);
	if (itTexture == m_textureTypeSlots.end())
	{
		return std::nullopt;
	}

	return itTexture->second;
}

}