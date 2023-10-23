#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"

#include <map>
#include <set>
#include <string>

namespace engine
{

class ShaderCollections final
{
public:
	static constexpr uint16_t InvalidProgramHandle = UINT16_MAX;

public:
	ShaderCollections() = default;
	ShaderCollections(const ShaderCollections&) = default;
	ShaderCollections& operator=(const ShaderCollections&) = default;
	ShaderCollections(ShaderCollections&&) = default;
	ShaderCollections& operator=(ShaderCollections&&) = default;
	~ShaderCollections() = default;

	void RegisterShaderProgram(StringCrc programNameCrc, std::initializer_list<std::string> names);

	void AddFeatureCombine(StringCrc programNameCrc, std::string combine);
	void DeleteFeatureCombine(StringCrc programNameCrc, std::string combine);

	void SetShaders(StringCrc programNameCrc, std::set<std::string> shaders);
	std::set<std::string>& GetShaders(StringCrc programNameCrc) { return m_shaderPrograms[programNameCrc.Value()]; }
	const std::set<std::string>& GetShaders(StringCrc programNameCrc) const { return m_shaderPrograms.at(programNameCrc.Value()); }

	void SetFeatureCombines(StringCrc programNameCrc, std::set<std::string> combine);
	std::set<std::string>& GetFeatureCombines(StringCrc programNameCrc) { return m_programFeatureCombines[programNameCrc.Value()]; }
	const std::set<std::string>& GetFeatureCombines(StringCrc programNameCrc) const { return m_programFeatureCombines.at(programNameCrc.Value()); }

	bool IsProgramValid(StringCrc programNameCrc) const;
	bool HasFeatureCombine(StringCrc programNameCrc) const;

	void SetShaderPrograms(std::map<uint32_t, std::set<std::string>> shaders);
	std::map<uint32_t, std::set<std::string>>& GetShaderPrograms() { return m_shaderPrograms; }
	const std::map<uint32_t, std::set<std::string>>& GetShaderPrograms() const { return m_shaderPrograms; }

	void SetFeatureCombinePrograms(std::map<uint32_t, std::set<std::string>> combines);
	std::map<uint32_t, std::set<std::string>>& GetFeatureCombinePrograms() { return m_programFeatureCombines; }
	const std::map<uint32_t, std::set<std::string>>& GetFeatureCombinePrograms() const { return m_programFeatureCombines; }

private:
	// Key : StringCrc(Program name), Value : Shader names
	std::map<uint32_t, std::set<std::string>> m_shaderPrograms;
	// Key : StringCrc(Program name), Value : Feature combines used as a parameter for compiling shaders
	std::map<uint32_t, std::set<std::string>> m_programFeatureCombines;
};

}