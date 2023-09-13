#include "ShaderVariantCollectionsComponent.h"

#include "Log/Log.h"

#include <cassert>

namespace engine
{

void ShaderVariantCollectionsComponent::RegisterPragram(std::string programName, ShaderProgramPack pack)
{
	if (IsValid(programName))
	{
		CD_ENGINE_WARN("Program pack {0} already exists in ShaderLibrary!", cd::MoveTemp(programName));
		return;
	}

	m_shaderPrograms[cd::MoveTemp(programName)] = cd::MoveTemp(pack);
}

void ShaderVariantCollectionsComponent::ActivateShaderFeature(std::string programName, ShaderFeature feature)
{
	assert(IsValid(programName) && "Program does not exist in ShaderLibrary!");

	m_shaderPrograms[cd::MoveTemp(programName)].ActivateShaderFeature(cd::MoveTemp(feature));
}

void ShaderVariantCollectionsComponent::DeactiveShaderFeature(std::string programName, ShaderFeature feature)
{
	assert(IsValid(programName) && "Shader information does not exist!");

	m_shaderPrograms[cd::MoveTemp(programName)].DeactivateShaderFeature(cd::MoveTemp(feature));
}

void ShaderVariantCollectionsComponent::SetShaderPrograms(std::map<std::string, ShaderProgramPack> program)
{
	m_shaderPrograms = cd::MoveTemp(program);
}

bool ShaderVariantCollectionsComponent::IsValid(std::string path) const
{
	return (m_shaderPrograms.find(cd::MoveTemp(path)) != m_shaderPrograms.end());
}

}