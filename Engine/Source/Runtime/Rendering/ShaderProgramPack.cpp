#include "ShaderProgramPack.h"

#include "Base/Template.h"

#include <cassert>

namespace engine
{

ShaderProgramPack::ShaderProgramPack(std::initializer_list<std::string> names)
{
	for (auto& name : names)
	{
		m_shaderNames.emplace_back(cd::MoveTemp(name));
	}
}

void ShaderProgramPack::AddShader(std::string path)
{
	m_shaderNames.emplace_back(cd::MoveTemp(path));
}

void ShaderProgramPack::ActivateShaderFeature(ShaderFeature feature)
{
	assert(!IsFeatureValid(feature) && "Shader feature already exists in ShaderProgramPack!");
	m_featureSet.insert(cd::MoveTemp(feature));
}

void ShaderProgramPack::DeactivateShaderFeature(ShaderFeature feature)
{
	assert(IsFeatureValid(feature) && "Shader feature does not exist in ShaderProgramPack!");
	m_featureSet.erase(cd::MoveTemp(feature));
}

void ShaderProgramPack::SetShaderNames(std::vector<std::string> names)
{
	m_shaderNames = cd::MoveTemp(names);
}

void ShaderProgramPack::SetFeatureSet(ShaderFeatureSet featureSet)
{
	m_featureSet = cd::MoveTemp(featureSet);
}

inline bool ShaderProgramPack::IsFeatureValid(ShaderFeature feature) const
{
	return (m_featureSet.find(feature) != m_featureSet.end());
}

}