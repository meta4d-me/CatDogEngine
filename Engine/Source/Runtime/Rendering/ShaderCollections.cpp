#include "ShaderCollections.h"

#include "Log/Log.h"

#include <cassert>

namespace engine
{

void ShaderCollections::RegisterShaderProgram(StringCrc programNameCrc, std::initializer_list<std::string> names)
{
	if (IsProgramValid(programNameCrc) || HasFeatureCombine(programNameCrc))
	{
		return;
	}
	
	m_shaderPrograms[programNameCrc] = cd::MoveTemp(names);
}

void ShaderCollections::AddFeatureCombine(StringCrc programNameCrc, std::string combine)
{
	assert(IsProgramValid(programNameCrc) && "Shader program does not exist!");

	m_programFeatureCombines[programNameCrc].insert(cd::MoveTemp(combine));
}

void ShaderCollections::DeleteFeatureCombine(StringCrc programNameCrc, std::string combine)
{
	assert(HasFeatureCombine(programNameCrc) && "Feature combine does not exist!");

	m_programFeatureCombines[programNameCrc].erase(cd::MoveTemp(combine));
}

void ShaderCollections::SetShaders(StringCrc programNameCrc, std::set<std::string> shaders)
{
	assert(IsProgramValid(programNameCrc) && "Shader program does not exist!");

	m_shaderPrograms[programNameCrc] = cd::MoveTemp(shaders);
}

void ShaderCollections::SetFeatureCombines(StringCrc programNameCrc, std::set<std::string> combine)
{
	assert(IsProgramValid(programNameCrc) && "Shader program does not exist!");

	m_programFeatureCombines[programNameCrc] = cd::MoveTemp(combine);
}

bool ShaderCollections::IsProgramValid(StringCrc programNameCrc) const
{
	return (m_shaderPrograms.find(programNameCrc) != m_shaderPrograms.end());
}

bool ShaderCollections::HasFeatureCombine(StringCrc programNameCrc) const
{
	return (m_programFeatureCombines.find(programNameCrc) != m_programFeatureCombines.end());
}

void ShaderCollections::SetShaderPrograms(std::map<StringCrc, std::set<std::string>> shaders)
{
	m_shaderPrograms = cd::MoveTemp(shaders);
}

void ShaderCollections::SetProgramFeatureCombines(std::map<StringCrc, std::set<std::string>> combines)
{
	m_programFeatureCombines = cd::MoveTemp(combines);
}

}