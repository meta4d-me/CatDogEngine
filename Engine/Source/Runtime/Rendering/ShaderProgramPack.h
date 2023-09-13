#pragma once

#include "Rendering/ShaderFeature.h"

#include <initializer_list>
#include <string>

namespace engine
{

class ShaderProgramPack
{
public:
	ShaderProgramPack(std::initializer_list<std::string> names);

	ShaderProgramPack() = default;
	ShaderProgramPack(const ShaderProgramPack&) = default;
	ShaderProgramPack& operator=(const ShaderProgramPack&) = default;
	ShaderProgramPack(ShaderProgramPack&&) = default;
	ShaderProgramPack& operator=(ShaderProgramPack&&) = default;
	~ShaderProgramPack() = default;

	void AddShader(std::string path);

	void ActivateShaderFeature(ShaderFeature feature);
	void DeactivateShaderFeature(ShaderFeature feature);

	void SetShaderNames(std::vector<std::string> names);
	std::vector<std::string>& GetShaderNames() { return m_shaderNames; }
	const std::vector<std::string>& GetShaderNames() const { return m_shaderNames; }

	void SetFeatureSet(ShaderFeatureSet featureSet);
	ShaderFeatureSet& GetFeatureSet() { return m_featureSet; }
	const ShaderFeatureSet& GetFeatureSet() const { return m_featureSet; }

private:
	inline bool IsFeatureValid(ShaderFeature feature) const;

	// TODO : Check is program legal, VS + FS or only CS will be a legal program.
	std::vector<std::string> m_shaderNames;
	ShaderFeatureSet m_featureSet;
};

}