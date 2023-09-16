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
	// static const ShaderType GetShaderType(const std::string& fileName);

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
	std::set<std::string>& GetFeatureCombines(const std::string& programName) { return m_featureCombinePrograms[programName]; }
	const std::set<std::string>& GetFeatureCombines(const std::string& programName) const { return m_featureCombinePrograms.at(programName); }

	void SetNonUberShaderProgramHandle(const std::string& programName, uint16_t handle) { m_nonUberShaderProgramHandles[programName] = handle; }
	uint16_t GetNonUberShaderProgramHandle(const std::string& programName) { return m_nonUberShaderProgramHandles[programName]; }
	const uint16_t GetNonUberShaderProgramHandle(const std::string& programName) const { return m_nonUberShaderProgramHandles.at(programName); }

	void SetUberShaderProgramHandle(const std::string& programName, const uint32_t featureCombineCrc, uint16_t handle) { m_uberShaderProgramHandles[programName][featureCombineCrc] = handle; }
	uint16_t GetUberShaderProgramHandle(const std::string& programName, const uint32_t featureCombineCrc) { return m_uberShaderProgramHandles[programName][featureCombineCrc]; }
	const uint16_t GetUberShaderProgramHandle(const std::string& programName, const uint32_t featureCombineCrc) const { return m_uberShaderProgramHandles.at(programName).at(featureCombineCrc); }

	void SetNonUberShaderPrograms(std::map<std::string, std::set<std::string>> shaders);
	std::map<std::string, std::set<std::string>>& GetNonUberShaderPrograms() { return m_nonUberShaderPrograms; }
	const std::map<std::string, std::set<std::string>>& GetNonUberShaderPrograms() const { return m_nonUberShaderPrograms; }

	void SetUberShaderPrograms(std::map<std::string, std::set<std::string>> shaders);
	std::map<std::string, std::set<std::string>>& GetUberShaderPrograms() { return m_uberShaderPrograms; }
	const std::map<std::string, std::set<std::string>>& GetUberShaderPrograms() const { return m_uberShaderPrograms; }

	void SetFeatureCombinePrograms(std::map<std::string, std::set<std::string>> combines);
	std::map<std::string, std::set<std::string>>& GetFeatureCombinePrograms() { return m_featureCombinePrograms; }
	const std::map<std::string, std::set<std::string>>& GetFeatureCombinePrograms() const { return m_featureCombinePrograms; }

	void SetNonUberShaderProgramHandles(std::map<std::string, uint16_t> handles);
	std::map<std::string, uint16_t>& GetNonUberShaderProgramHandles() { return m_nonUberShaderProgramHandles; }
	const std::map<std::string, uint16_t>& GetNonUberShaderProgramHandles() const { return m_nonUberShaderProgramHandles; }

	void SetUberShaderProgramHandles(std::map<std::string, std::map<uint32_t, uint16_t>> handles);
	std::map<std::string, std::map<uint32_t, uint16_t>>& GetUberShaderProgramHandles() { return m_uberShaderProgramHandles; }
	const std::map<std::string, std::map<uint32_t, uint16_t>>& GetUberShaderProgramHandles() const { return m_uberShaderProgramHandles; }

private:
	inline bool IsNonUberShaderProgramValid(std::string programName) const;
	inline bool IsUberShaderProgramValid(std::string programName) const;

	// Key : Program name, Value : Non-uber shader names
	std::map<std::string, std::set<std::string>> m_nonUberShaderPrograms;

	// Key : Program name, Value : Uber shader names
	std::map<std::string, std::set<std::string>> m_uberShaderPrograms;
	// Key : Program name, Value : Feature combine used as a parameter for compiling shaders
	std::map<std::string, std::set<std::string>> m_featureCombinePrograms;

	// Key : Program name, Value : Non-uber hader shader program handle
	std::map<std::string, uint16_t> m_nonUberShaderProgramHandles;
	// Key : Program name, Value : {Key : feature combine, Value : Uber hader shader program handle}
	std::map<std::string, std::map<uint32_t, uint16_t>> m_uberShaderProgramHandles;

	// TODO : StringCrc
	// TODO : Check is program legal, VS + FS or only CS will be a legal program.
};

}