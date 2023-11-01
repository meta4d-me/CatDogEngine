#pragma once

#include "Core/StringCrc.h"
#include "Material/ShaderSchema.h"
#include "Scene/VertexFormat.h"
#include "Scene/MaterialTextureType.h"

#include <map>
#include <optional>
#include <set>
#include <string>

namespace engine
{

// Most used in the editor level to define different material types so that raw material asset data can
// map to a specified MaterialType.
class MaterialType
{
public:
	MaterialType() = default;
	MaterialType(const MaterialType&) = delete;
	MaterialType& operator=(const MaterialType&) = delete;
	MaterialType(MaterialType&&) = default;
	MaterialType& operator=(MaterialType&&) = default;
	~MaterialType() = default;

	const char* GetMaterialName() const { return m_materialName.c_str(); }
	void SetMaterialName(std::string materialName) { m_materialName = cd::MoveTemp(materialName); }

	ShaderSchema& GetShaderSchema() { return m_shaderSchema; }
	const ShaderSchema& GetShaderSchema() const { return m_shaderSchema; }
	void SetShaderSchema(ShaderSchema shaderSchema) { m_shaderSchema = cd::MoveTemp(shaderSchema); }

	void SetRequiredVertexFormat(cd::VertexFormat vertexFormat) { m_requiredVertexFormat = cd::MoveTemp(vertexFormat); }
	const cd::VertexFormat& GetRequiredVertexFormat() const { return m_requiredVertexFormat; }

	void AddOptionalTextureType(cd::MaterialTextureType textureType, uint8_t slot);
	const std::set<cd::MaterialTextureType>& GetOptionalTextureTypes() const { return m_optionalTextureTypes; }

	std::optional<uint8_t> GetTextureSlot(cd::MaterialTextureType textureType) const;

private:
	std::string m_materialName;
	ShaderSchema m_shaderSchema;

	cd::VertexFormat m_requiredVertexFormat;
	std::set<cd::MaterialTextureType> m_optionalTextureTypes;
	std::map<cd::MaterialTextureType, uint8_t> m_textureTypeSlots;
};

}