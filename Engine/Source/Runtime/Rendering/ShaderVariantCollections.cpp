#include "ShaderVariantCollections.h"

#include "Log/Log.h"

#include <cassert>

namespace engine
{

void ShaderVariantCollections::RegisterNonUberShader(std::string programName, std::initializer_list<std::string> names)
{
	if (IsNonUberShaderProgramValid(programName))
	{
		CD_ENGINE_WARN("Non uber shader program {0} already exists!", programName);
		return;
	}

	m_nonUberShaderPrograms[cd::MoveTemp(programName)] = cd::MoveTemp(names);
}

void ShaderVariantCollections::RegisterUberShader(std::string programName, std::initializer_list<std::string> names, std::initializer_list<std::string> combines)
{
	if (IsUberShaderProgramValid(programName))
	{
		CD_ENGINE_WARN("Uber shader program {0} already exists!", programName);
		return;
	}

	m_uberShaderPrograms[cd::MoveTemp(programName)] = cd::MoveTemp(names);
	m_featureCombinePrograms[cd::MoveTemp(programName)] = cd::MoveTemp(combines);
}

void ShaderVariantCollections::AddFeatureCombine(std::string programName, std::string combine)
{
	assert(IsUberShaderProgramValid(programName) && "Uber shader does not exist!");

	m_featureCombinePrograms[cd::MoveTemp(programName)].insert(cd::MoveTemp(combine));
}

void ShaderVariantCollections::DeleteFeatureCombine(std::string programName, std::string combine)
{
	assert(IsUberShaderProgramValid(programName) && "Uber shader does not exist!");

	m_featureCombinePrograms[cd::MoveTemp(programName)].erase(cd::MoveTemp(combine));
}

void ShaderVariantCollections::SetNonUberShaders(const std::string& programName, std::set<std::string> shaders)
{
	m_nonUberShaderPrograms[programName] = cd::MoveTemp(shaders);
}

void ShaderVariantCollections::SetUberShaders(const std::string& programName, std::set<std::string> shaders)
{
	m_uberShaderPrograms[programName] = cd::MoveTemp(shaders);
}

void ShaderVariantCollections::SetFeatureCombines(const std::string& programName, std::set<std::string> combine)
{
	m_featureCombinePrograms[programName] = cd::MoveTemp(combine);
}

void ShaderVariantCollections::SetNonUberShaderPrograms(std::map<std::string, std::set<std::string>> shaders)
{
	m_nonUberShaderPrograms = cd::MoveTemp(shaders);
}

void ShaderVariantCollections::SetUberShaderPrograms(std::map<std::string, std::set<std::string>> shaders)
{
	m_uberShaderPrograms = cd::MoveTemp(shaders);
}

void ShaderVariantCollections::SetFeatureCombinePrograms(std::map<std::string, std::set<std::string>> combines)
{
	m_featureCombinePrograms = cd::MoveTemp(combines);
}

void ShaderVariantCollections::SetNonUberShaderProgramHandles(std::map<std::string, uint16_t> handles)
{
	m_nonUberShaderProgramHandles = cd::MoveTemp(handles);
}

void ShaderVariantCollections::SetUberShaderProgramHandles(std::map<std::string, std::map<uint32_t, uint16_t>> handles)
{
	m_uberShaderProgramHandles = cd::MoveTemp(handles);
}

bool ShaderVariantCollections::IsNonUberShaderProgramValid(std::string programName) const
{
	return (m_nonUberShaderPrograms.find(cd::MoveTemp(programName)) != m_nonUberShaderPrograms.end());
}

bool ShaderVariantCollections::IsUberShaderProgramValid(std::string programName) const
{
	return (m_uberShaderPrograms.find(cd::MoveTemp(programName)) != m_uberShaderPrograms.end() && 
		m_featureCombinePrograms.find(cd::MoveTemp(programName)) != m_featureCombinePrograms.end());
}

}