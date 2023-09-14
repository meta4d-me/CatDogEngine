#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Rendering/ShaderProgramPack.h"

#include <map>
#include <string>

namespace engine
{

class ShaderVariantCollections final
{
public:
	ShaderVariantCollections() = default;
	ShaderVariantCollections(const ShaderVariantCollections&) = default;
	ShaderVariantCollections& operator=(const ShaderVariantCollections&) = default;
	ShaderVariantCollections(ShaderVariantCollections&&) = default;
	ShaderVariantCollections& operator=(ShaderVariantCollections&&) = default;
	~ShaderVariantCollections() = default;

	void RegisterPragram(std::string programName, ShaderProgramPack pack);

	void ActivateShaderFeature(std::string programName, ShaderFeature feature);
	void DeactiveShaderFeature(std::string programName, ShaderFeature feature);

	void SetShaderPrograms(std::map<std::string, ShaderProgramPack> program);
	std::map<std::string, ShaderProgramPack>& GetShaderPrograms() { return m_shaderPrograms; }
	const std::map<std::string, ShaderProgramPack>& GetShaderPrograms() const { return m_shaderPrograms; }

private:
	inline bool IsProgramValid(std::string programName) const;

	// Key : Program Name, Value : Shader Path + Variant.
	// TODO : StringCrc
	std::map<std::string, ShaderProgramPack> m_shaderPrograms;
};

}