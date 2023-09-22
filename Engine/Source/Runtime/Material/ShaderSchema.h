#pragma once

#include "Core/StringCrc.h"
#include "Rendering/ShaderFeature.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace engine
{

enum class LoadingStatus : uint8_t
{
	MISSING_RESOURCES = 0,
	LOADING_SHADERS,
	LOADING_TEXTURES,
	LOADING_ERROR,

	COUNT,
};

class ShaderSchema
{
public:
	static constexpr uint16_t InvalidProgramHandle = UINT16_MAX;
	static constexpr StringCrc DefaultUberShaderCrc = StringCrc("");
	using ShaderBlob = std::vector<std::byte>;

public:
	ShaderSchema() = default;
	explicit ShaderSchema(std::string progeamName, std::string vsPath, std::string fsPath);
	ShaderSchema(const ShaderSchema&) = delete;
	ShaderSchema& operator=(const ShaderSchema&) = delete;
	ShaderSchema(ShaderSchema&&) = default;
	ShaderSchema& operator=(ShaderSchema&&) = default;
	~ShaderSchema() = default;

	const char* GetProgramName() const { return m_programName.c_str(); }
	const char* GetVertexShaderPath() const { return m_vertexShaderPath.c_str(); }
	const char* GetFragmentShaderPath() const { return m_fragmentShaderPath.c_str(); }

	void AddFeatureSet(ShaderFeatureSet featureSet);

	void Build();
	void CleanBuild();
	void CleanAll();

	std::string GetFeaturesCombine(const ShaderFeatureSet& featureSet) const;
	StringCrc GetFeaturesCombineCrc(const ShaderFeatureSet& featureSet) const;

	std::set<ShaderFeatureSet>& GetFeatures() { return m_shaderFeatureSets; }
	const std::set<ShaderFeatureSet>& GetFeatures() const { return m_shaderFeatureSets; }

	std::set<std::string>& GetAllFeatureCombines() { return m_allFeatureCombines; }
	const std::set<std::string>& GetAllFeatureCombines() const { return m_allFeatureCombines; }

private:
	std::string m_programName;
	std::string m_vertexShaderPath;
	std::string m_fragmentShaderPath;

	bool m_isDirty = false;
	// Registration order of shader features.
	std::set<ShaderFeatureSet> m_shaderFeatureSets;
	// All permutations matching the registered shader features.
	std::set<std::string> m_allFeatureCombines;
};

}