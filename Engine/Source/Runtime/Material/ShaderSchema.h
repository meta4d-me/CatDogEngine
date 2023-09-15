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

	// Calling "AddFeatureSet/SetConflictOptions and Build" after Build will cause unnecessary performance overhead.
	void Build();
	void CleanBuild();
	void CleanAll();

	void SetCompiledProgram(StringCrc shaderFeaturesCrc, uint16_t programHandle);
	uint16_t GetCompiledProgram(StringCrc shaderFeaturesCrc) const;

	StringCrc GetFeaturesCrc(const ShaderFeatureSet& featureSet) const;
	bool IsFeaturesValid(StringCrc shaderFeaturesCrc) const;

	std::vector<ShaderFeatureSet>& GetFeatures() { return m_shaderFeatureSets; }
	const std::vector<ShaderFeatureSet>& GetFeatures() const { return m_shaderFeatureSets; }

	std::vector<std::string>& GetFeatureCombines() { return m_featureCombines; }
	const std::vector<std::string>& GetFeatureCombines() const { return m_featureCombines; }

	// TODO : More generic.
	void AddUberVSBlob(ShaderBlob shaderBlob);
	void AddUberFSBlob(StringCrc shaderFeaturesCrc, ShaderBlob shaderBlob);
	const ShaderBlob& GetVSBlob() const { return *m_pVSBlob.get(); }
	const ShaderBlob& GetFSBlob(StringCrc shaderFeaturesCrc) const;

private:
	std::string m_programName;

	std::string m_vertexShaderPath;
	std::string m_fragmentShaderPath;

	bool m_isDirty = false;
	// Adding order of shader features.
	std::vector<ShaderFeatureSet> m_shaderFeatureSets;
	// Parameters to compile shaders.
	std::vector<std::string> m_featureCombines;
	// Key: StringCrc(feature combine), Value: shader handle.
	std::map<uint32_t, uint16_t> m_compiledProgramHandles;

	std::unique_ptr<ShaderBlob> m_pVSBlob;
	std::map<uint32_t, std::unique_ptr<ShaderBlob>> m_shaderFeaturesToFSBlobs;
};

}