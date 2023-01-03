#pragma once

#include "Core/StringCrc.h"
#include "Scene/VertexFormat.h"

#include <map>
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

	void SetVertexFormat(cd::VertexFormat vertexFormat) { m_requiredVertexFormat = cd::MoveTemp(vertexFormat); }
	const cd::VertexFormat& GetVertexFormat() const { return m_requiredVertexFormat; }

private:
	std::string m_materialName;
	std::string m_vertexShaderName;
	std::string m_fragmentShaderName;

	cd::VertexFormat m_requiredVertexFormat;

	// Parameter types
	std::vector<std::string> m_parameterTypes;

	// Texture types
	std::map<std::string, std::string> m_textureTypes;

	// Samplers
	//std::map<std::string, Sampler> m_samplers;
};

}