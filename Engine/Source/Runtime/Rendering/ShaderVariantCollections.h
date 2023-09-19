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

	void RegisterNonUberShader(std::string programName, std::initializer_list<std::string> names);
	void RegisterUberShader(std::string programName, std::initializer_list<std::string> names, std::initializer_list<std::string> combines = {});

	void AddFeatureCombine(std::string programName, std::string combine);
	void DeleteFeatureCombine(std::string programName, std::string combine);

	void SetNonUberShaders(const std::string& programName, std::set<std::string> shaders);
	std::set<std::string>& GetNonUberShaders(const std::string& programName) { return m_nonUberShaderPrograms[programName]; }
	const std::set<std::string>& GetNonUberShaders(const std::string& programName) const { return m_nonUberShaderPrograms.at(programName); }

	void SetUberShaders(const std::string& programName, std::set<std::string> shaders);
	std::set<std::string>& GetUberShaders(const std::string& programName) { return m_uberShaderPrograms[programName]; }
	const std::set<std::string>& GetUberShaders(const std::string& programName) const { return m_uberShaderPrograms.at(programName); }

	void SetFeatureCombines(const std::string& programName, std::set<std::string> combine);
	std::set<std::string>& GetFeatureCombines(const std::string& programName) { return m_programFeatureCombines[programName]; }
	const std::set<std::string>& GetFeatureCombines(const std::string& programName) const { return m_programFeatureCombines.at(programName); }

	bool IsUber(const std::string& programName);

	// -------------------------------------------------------------------------------- // 

	void SetNonUberShaderPrograms(std::map<std::string, std::set<std::string>> shaders);
	std::map<std::string, std::set<std::string>>& GetNonUberShaderPrograms() { return m_nonUberShaderPrograms; }
	const std::map<std::string, std::set<std::string>>& GetNonUberShaderPrograms() const { return m_nonUberShaderPrograms; }

	void SetUberShaderPrograms(std::map<std::string, std::set<std::string>> shaders);
	std::map<std::string, std::set<std::string>>& GetUberShaderPrograms() { return m_uberShaderPrograms; }
	const std::map<std::string, std::set<std::string>>& GetUberShaderPrograms() const { return m_uberShaderPrograms; }

	void SetFeatureCombinePrograms(std::map<std::string, std::set<std::string>> combines);
	std::map<std::string, std::set<std::string>>& GetFeatureCombinePrograms() { return m_programFeatureCombines; }
	const std::map<std::string, std::set<std::string>>& GetFeatureCombinePrograms() const { return m_programFeatureCombines; }

private:
	inline bool IsNonUberShaderProgramValid(std::string programName) const;
	inline bool IsUberShaderProgramValid(std::string programName) const;

	// Key : Program name, Value : Non-uber shader names
	std::map<std::string, std::set<std::string>> m_nonUberShaderPrograms;

	// Key : Program name, Value : Uber shader names
	std::map<std::string, std::set<std::string>> m_uberShaderPrograms;
	// Key : Program name, Value : Feature combine used as a parameter for compiling shaders
	std::map<std::string, std::set<std::string>> m_programFeatureCombines;

	// TODO : StringCrc
};

}