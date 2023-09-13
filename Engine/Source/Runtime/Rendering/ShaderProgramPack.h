#pragma once

#include "Rendering/ShaderFeature.h"

#include <initializer_list>
#include <string>

namespace engine
{

class ShaderProgramPack
{
public:
	ShaderProgramPack(std::initializer_list<std::string> paths);

	ShaderProgramPack() = default;
	ShaderProgramPack(const ShaderProgramPack&) = default;
	ShaderProgramPack& operator=(const ShaderProgramPack&) = default;
	ShaderProgramPack(ShaderProgramPack&&) = default;
	ShaderProgramPack& operator=(ShaderProgramPack&&) = default;
	~ShaderProgramPack() = default;

	void AddShader(std::string path);

	void ActivateShaderFeature(ShaderFeature feature);
	void DeactivateShaderFeature(ShaderFeature feature);

	void SetShaderPaths(std::vector<std::string> paths);
	std::vector<std::string>& GetShaderPaths() { return m_shaderPaths; }
	const std::vector<std::string>& GetShaderPaths() const { return m_shaderPaths; }

	void SetFeatureSet(ShaderFeatureSet featureSet);
	ShaderFeatureSet& GetFeatureSet() { return m_featureSet; }
	const ShaderFeatureSet& GetFeatureSet() const { return m_featureSet; }

private:
	// TODO : Check is program legal, VS + FS or only CS will be a legal program.
	std::vector<std::string> m_shaderPaths;
	ShaderFeatureSet m_featureSet;
};

}