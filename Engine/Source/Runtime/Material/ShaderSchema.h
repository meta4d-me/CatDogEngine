#pragma once

#include "Core/StringCrc.h"
#include "Base/Template.h"
#include "Rendering/ShaderFeature.h"

#include <map>
#include <memory>
#include <optional>
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
	ShaderSchema(const ShaderSchema&) = delete;
	ShaderSchema& operator=(const ShaderSchema&) = delete;
	ShaderSchema(ShaderSchema&&) = default;
	ShaderSchema& operator=(ShaderSchema&&) = default;
	~ShaderSchema() = default;

	void SetShaderProgramName(std::string name) { m_shaderProgramName = cd::MoveTemp(name); }
	std::string& GetShaderProgramName() { return m_shaderProgramName; }
	const std::string& GetShaderProgramName() const { return m_shaderProgramName; }

	void Build();
	void CleanBuild();
	void CleanAll();

	void AddFeatureSet(ShaderFeatureSet featureSet);
	std::optional<ShaderFeatureSet> GetConflictFeatureSet(const ShaderFeature feature) const;
	std::string GetFeaturesCombine(const ShaderFeatureSet& featureSet) const;

	std::set<ShaderFeatureSet>& GetFeatureSets() { return m_shaderFeatureSets; }
	const std::set<ShaderFeatureSet>& GetFeatureSets() const { return m_shaderFeatureSets; }

	std::set<std::string>& GetAllFeatureCombines();
	const std::set<std::string>& GetAllFeatureCombines() const;

private:
	std::string m_shaderProgramName;

	bool m_isDirty = false;
	// Registration order of shader feature sets.
	std::set<ShaderFeatureSet> m_shaderFeatureSets;
	// All permutations matching the registered shader features.
	std::set<std::string> m_allFeatureCombines;
};

}