#include "ShaderVariantCollections.h"

#include "Log/Log.h"

#include <cassert>

namespace engine
{

void ShaderVariantCollections::RegisterShaderProgram(StringCrc programNameCrc, std::initializer_list<std::string> names)
{
	if (IsProgramValid(programNameCrc) || HasFeatureCombine(programNameCrc))
	{
		CD_ENGINE_WARN("Shader program already exists!");
		return;
	}
	
	m_shaderPrograms[programNameCrc.Value()] = cd::MoveTemp(names);
}

void ShaderVariantCollections::AddFeatureCombine(StringCrc programNameCrc, std::string combine)
{
	assert(IsProgramValid(programNameCrc) && "Shader program does not exist!");

	m_programFeatureCombines[programNameCrc.Value()].insert(cd::MoveTemp(combine));
}

void ShaderVariantCollections::DeleteFeatureCombine(StringCrc programNameCrc, std::string combine)
{
	assert(HasFeatureCombine(programNameCrc) && "Feature combine does not exist!");

	m_programFeatureCombines[programNameCrc.Value()].erase(cd::MoveTemp(combine));
}

void ShaderVariantCollections::SetShaders(StringCrc programNameCrc, std::set<std::string> shaders)
{
	assert(IsProgramValid(programNameCrc) && "Shader program does not exist!");

	m_shaderPrograms[programNameCrc.Value()] = cd::MoveTemp(shaders);
}

void ShaderVariantCollections::SetFeatureCombines(StringCrc programNameCrc, std::set<std::string> combine)
{
	assert(IsProgramValid(programNameCrc) && "Shader program does not exist!");

	m_programFeatureCombines[programNameCrc.Value()] = cd::MoveTemp(combine);
}

bool ShaderVariantCollections::IsProgramValid(StringCrc programNameCrc) const
{
	return (m_shaderPrograms.find(programNameCrc.Value()) != m_shaderPrograms.end());
}

bool ShaderVariantCollections::HasFeatureCombine(StringCrc programNameCrc) const
{
	return (m_programFeatureCombines.find(programNameCrc.Value()) != m_programFeatureCombines.end());
}

void ShaderVariantCollections::SetShaderPrograms(std::map<uint32_t, std::set<std::string>> shaders)
{
	m_shaderPrograms = cd::MoveTemp(shaders);
}

void ShaderVariantCollections::SetFeatureCombinePrograms(std::map<uint32_t, std::set<std::string>> combines)
{
	m_programFeatureCombines = cd::MoveTemp(combines);
}

}