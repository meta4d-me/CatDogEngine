#pragma once

#include "Core/StringCrc.h"
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
	static MaterialType GetPBRMaterialType();

public:
	MaterialType() = default;
	MaterialType(const MaterialType&) = default;
	MaterialType& operator=(const MaterialType&) = default;
	MaterialType(MaterialType&&) = default;
	MaterialType& operator=(MaterialType&&) = default;
	~MaterialType() = default;

	const char* GetMaterialName() const { return m_materialName.c_str(); }
	const char* GetVertexShaderName() const { return m_vertexShaderName.c_str(); }
	const char* GetFragmentShaderName() const { return m_fragmentShaderName.c_str(); }

	void SetRequiredVertexFormat(cd::VertexFormat vertexFormat) { m_requiredVertexFormat = cd::MoveTemp(vertexFormat); }
	const cd::VertexFormat& GetRequiredVertexFormat() const { return m_requiredVertexFormat; }

	void AddOptionalTextureType(cd::MaterialTextureType textureType, uint8_t slot);
	const std::set<cd::MaterialTextureType>& GetOptionalTextureTypes() const { return m_optionalTextureTypes; }

	void AddRequiredTextureType(cd::MaterialTextureType textureType, uint8_t slot);
	const std::set<cd::MaterialTextureType>& GetRequiredTextureTypes() const { return m_requiredTextureTypes; }

	std::optional<uint8_t> GetTextureSlot(cd::MaterialTextureType textureType) const;

private:
	std::string m_materialName;
	std::string m_vertexShaderName;
	std::string m_fragmentShaderName;

	cd::VertexFormat m_requiredVertexFormat;
	std::set<cd::MaterialTextureType> m_optionalTextureTypes;
	std::set<cd::MaterialTextureType> m_requiredTextureTypes;
	std::map<cd::MaterialTextureType, uint8_t> m_textureTypeSlots;
};

}