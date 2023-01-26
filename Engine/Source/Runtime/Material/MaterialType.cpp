#include "MaterialType.h"

#include "Math/Vector.hpp"

namespace
{

std::string GetShaderPath(const char* pShaderName)
{
	std::string shaderPath = CDENGINE_BUILTIN_SHADER_PATH;
	shaderPath += pShaderName;
	shaderPath += ".sc";

	return shaderPath;
}

}

namespace engine
{

MaterialType MaterialType::GetPBRMaterialType()
{
	ShaderSchema shaderSchema(GetShaderPath("vs_PBR"), GetShaderPath("fs_PBR"));
	shaderSchema.RegisterUberOption(ShaderSchema::DefaultUberOptionName);
	shaderSchema.RegisterUberOption("USE_PBR_IBL");

	MaterialType pbr;
	pbr.m_materialName = "CD_PBR";
	pbr.m_shaderSchema = cd::MoveTemp(shaderSchema);

	cd::VertexFormat pbrVertexFormat;
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
	pbrVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
	pbr.SetRequiredVertexFormat(cd::MoveTemp(pbrVertexFormat));

	// Slot index should align to shader codes.
	pbr.AddRequiredTextureType(cd::MaterialTextureType::BaseColor, 0);
	pbr.AddRequiredTextureType(cd::MaterialTextureType::Normal, 1);

	pbr.AddOptionalTextureType(cd::MaterialTextureType::AO, 2);
	pbr.AddOptionalTextureType(cd::MaterialTextureType::Metalness, 2);
	pbr.AddOptionalTextureType(cd::MaterialTextureType::Roughness, 2);

	//pbr.AddOptionalTextureType(cd::MaterialTextureType::Emissive, );

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

std::optional<uint8_t> MaterialType::GetTextureSlot(cd::MaterialTextureType textureType) const
{
	auto itTexture = m_textureTypeSlots.find(textureType);
	if (itTexture == m_textureTypeSlots.end())
	{
		return std::nullopt;
	}

	return itTexture->second;
}

}