#include "ShaderVariantCollectionsComponent.h"

#include "Log/Log.h"

#include <cassert>

namespace engine
{

void ShaderVariantCollectionsComponent::RegisterPragram(std::string programName, ShaderProgramPack pack)
{
	if (IsProgramValid(programName))
	{
		CD_ENGINE_WARN("Program pack {0} already exists in SVC!", cd::MoveTemp(programName));
		return;
	}

	m_shaderPrograms[cd::MoveTemp(programName)] = cd::MoveTemp(pack);
}

void ShaderVariantCollectionsComponent::ActivateShaderFeature(std::string programName, ShaderFeature feature)
{
	assert(IsProgramValid(programName) && "Program does not exist in SVC!");

	m_shaderPrograms[cd::MoveTemp(programName)].ActivateShaderFeature(cd::MoveTemp(feature));
}

void ShaderVariantCollectionsComponent::DeactiveShaderFeature(std::string programName, ShaderFeature feature)
{
	assert(IsProgramValid(programName) && "Program does not exist in SVC!");

	m_shaderPrograms[cd::MoveTemp(programName)].DeactivateShaderFeature(cd::MoveTemp(feature));
}

void ShaderVariantCollectionsComponent::SetShaderPrograms(std::map<std::string, ShaderProgramPack> program)
{
	m_shaderPrograms = cd::MoveTemp(program);
}

bool ShaderVariantCollectionsComponent::IsProgramValid(std::string programName) const
{
	return (m_shaderPrograms.find(cd::MoveTemp(programName)) != m_shaderPrograms.end());
}

}