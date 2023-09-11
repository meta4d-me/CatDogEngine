#include "ShaderVariantCollectionsComponent.h"

#include "Log/Log.h"

namespace engine
{

void ShaderVariantCollectionsComponent::AddShader(std::string path, ShaderFeatureSet set)
{
	if (IsValid(path))
	{
		CD_ENGINE_WARN("Shader information {0} already exists!", cd::MoveTemp(path));
		return;
	}

	m_shaderInformations[cd::MoveTemp(path)] = cd::MoveTemp(set);
}

void ShaderVariantCollectionsComponent::ActiveShaderFeature(std::string path, ShaderFeature feature)
{
	assert(IsValid(path) && "Shader information does not exist!");

	m_shaderInformations[cd::MoveTemp(path)].insert(cd::MoveTemp(feature));
}

void ShaderVariantCollectionsComponent::DeactiveShaderFeature(std::string path, ShaderFeature feature)
{
	assert(IsValid(path) && "Shader information does not exist!");

	m_shaderInformations[cd::MoveTemp(path)].erase(cd::MoveTemp(feature));
}

void ShaderVariantCollectionsComponent::SetShaderFeatureSet(std::string path, ShaderFeatureSet set)
{
	assert(IsValid(path) && "Shader information does not exist!");

	m_shaderInformations[cd::MoveTemp(path)] = cd::MoveTemp(set);
}

ShaderFeatureSet& ShaderVariantCollectionsComponent::GetShaderFeatureSet(std::string path)
{
	assert(IsValid(path) && "Shader information does not exist!");

	return m_shaderInformations[cd::MoveTemp(path)];
}

const ShaderFeatureSet& ShaderVariantCollectionsComponent::GetShaderFeatureSet(std::string path) const
{
	assert(IsValid(path) && "Shader information does not exist!");
	
	return m_shaderInformations.at(cd::MoveTemp(path));
}

void ShaderVariantCollectionsComponent::SetShaderInformations(std::map<std::string, ShaderFeatureSet> info)
{
	m_shaderInformations = cd::MoveTemp(info);
}

bool ShaderVariantCollectionsComponent::IsValid(std::string path) const
{
	return (m_shaderInformations.find(cd::MoveTemp(path)) != m_shaderInformations.end());
}

}