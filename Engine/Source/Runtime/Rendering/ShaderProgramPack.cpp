#include "ShaderProgramPack.h"

#include "Base/Template.h"

#include <cassert>

namespace engine
{

ShaderProgramPack::ShaderProgramPack(std::initializer_list<std::string> paths)
{
	for (auto& path : paths)
	{
		m_shaderPaths.emplace_back(cd::MoveTemp(path));
	}
}

void ShaderProgramPack::AddShader(std::string path)
{
	m_shaderPaths.emplace_back(cd::MoveTemp(path));
}

void ShaderProgramPack::ActivateShaderFeature(ShaderFeature feature)
{
	assert(m_featureSet.find(feature) == m_featureSet.end() && "Shader feature already exists in ShaderProgramPack!");
	m_featureSet.insert(cd::MoveTemp(feature));
}

void ShaderProgramPack::DeactivateShaderFeature(ShaderFeature feature)
{
	assert(m_featureSet.find(feature) != m_featureSet.end() && "Shader feature does not exist in ShaderProgramPack!");
	m_featureSet.erase(cd::MoveTemp(feature));
}

void ShaderProgramPack::SetShaderPaths(std::vector<std::string> paths)
{
	m_shaderPaths = cd::MoveTemp(paths);
}

void ShaderProgramPack::SetFeatureSet(ShaderFeatureSet featureSet)
{
	m_featureSet = cd::MoveTemp(featureSet);
}

}