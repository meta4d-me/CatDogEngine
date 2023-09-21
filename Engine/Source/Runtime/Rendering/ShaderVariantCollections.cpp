#include "ShaderVariantCollections.h"

#include "Log/Log.h"

#include <cassert>

namespace engine
{

void ShaderVariantCollections::RegisterShaderProgram(const std::string& programName, std::initializer_list<std::string> names)
{
	if (IsProgramValid(programName) || HasFeatureCombine(programName))
	{
		CD_ENGINE_WARN("Shader program {0} already exists!", programName);
		return;
	}
	
	m_shaderPrograms[programName] = cd::MoveTemp(names);
}

void ShaderVariantCollections::AddFeatureCombine(const std::string& programName, std::string combine)
{
	assert(IsProgramValid(programName) && "Shader program does not exist!");

	m_programFeatureCombines[programName].insert(cd::MoveTemp(combine));
}

void ShaderVariantCollections::DeleteFeatureCombine(const std::string& programName, std::string combine)
{
	assert(HasFeatureCombine(programName) && "Feature combine does not exist!");

	m_programFeatureCombines[programName].erase(cd::MoveTemp(combine));
}

void ShaderVariantCollections::SetShaders(const std::string& programName, std::set<std::string> shaders)
{
	assert(IsProgramValid(programName) && "Shader program does not exist!");

	m_shaderPrograms[programName] = cd::MoveTemp(shaders);
}

void ShaderVariantCollections::SetFeatureCombines(const std::string& programName, std::set<std::string> combine)
{
	assert(IsProgramValid(programName) && "Shader program does not exist!");

	m_programFeatureCombines[programName] = cd::MoveTemp(combine);
}

bool ShaderVariantCollections::IsProgramValid(const std::string& programName) const
{
	return (m_shaderPrograms.find(programName) != m_shaderPrograms.end());
}

bool ShaderVariantCollections::HasFeatureCombine(const std::string& programName) const
{
	return (m_programFeatureCombines.find(programName) != m_programFeatureCombines.end());
}

void ShaderVariantCollections::SetShaderPrograms(std::map<std::string, std::set<std::string>> shaders)
{
	m_shaderPrograms = cd::MoveTemp(shaders);
}

void ShaderVariantCollections::SetFeatureCombinePrograms(std::map<std::string, std::set<std::string>> combines)
{
	m_programFeatureCombines = cd::MoveTemp(combines);
}

}