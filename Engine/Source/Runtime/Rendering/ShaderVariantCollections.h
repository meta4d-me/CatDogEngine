#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"

#include <map>
#include <set>
#include <string>

namespace engine
{

class ShaderVariantCollections final
{
public:
	static constexpr uint16_t InvalidProgramHandle = UINT16_MAX;

public:
	ShaderVariantCollections() = default;
	ShaderVariantCollections(const ShaderVariantCollections&) = default;
	ShaderVariantCollections& operator=(const ShaderVariantCollections&) = default;
	ShaderVariantCollections(ShaderVariantCollections&&) = default;
	ShaderVariantCollections& operator=(ShaderVariantCollections&&) = default;
	~ShaderVariantCollections() = default;

	void RegisterShaderProgram(const std::string& programName, std::initializer_list<std::string> names);

	void AddFeatureCombine(const std::string& programName, std::string combine);
	void DeleteFeatureCombine(const std::string& programName, std::string combine);

	void SetShaders(const std::string& programName, std::set<std::string> shaders);
	std::set<std::string>& GetShaders(const std::string& programName) { return m_shaderPrograms[programName]; }
	const std::set<std::string>& GetShaders(const std::string& programName) const { return m_shaderPrograms.at(programName); }

	void SetFeatureCombines(const std::string& programName, std::set<std::string> combine);
	std::set<std::string>& GetFeatureCombines(const std::string& programName) { return m_programFeatureCombines[programName]; }
	const std::set<std::string>& GetFeatureCombines(const std::string& programName) const { return m_programFeatureCombines.at(programName); }

	bool IsProgramValid(const std::string& programName) const;
	bool HasFeatureCombine(const std::string& programName) const;

	// -------------------------------------------------------------------------------- // 

	void SetShaderPrograms(std::map<std::string, std::set<std::string>> shaders);
	std::map<std::string, std::set<std::string>>& GetShaderPrograms() { return m_shaderPrograms; }
	const std::map<std::string, std::set<std::string>>& GetShaderPrograms() const { return m_shaderPrograms; }

	void SetFeatureCombinePrograms(std::map<std::string, std::set<std::string>> combines);
	std::map<std::string, std::set<std::string>>& GetFeatureCombinePrograms() { return m_programFeatureCombines; }
	const std::map<std::string, std::set<std::string>>& GetFeatureCombinePrograms() const { return m_programFeatureCombines; }

private:
	// Key : Program name, Value : Shader names
	std::map<std::string, std::set<std::string>> m_shaderPrograms;
	// Key : Program name, Value : Feature combine used as a parameter for compiling shaders
	std::map<std::string, std::set<std::string>> m_programFeatureCombines;

	// TODO : StringCrc
};

}