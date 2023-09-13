#pragma once

#include "Core/StringCrc.h"
#include "Rendering/ShaderFeature.h"

#include <map>
#include <string>
#include <vector>

namespace engine
{

class ShaderVariant
{
public:
	static constexpr uint16_t InvalidProgramHandle = UINT16_MAX;
	static constexpr StringCrc DefaultUberShaderCrc = StringCrc("");

public:
	ShaderVariant() = default;
	ShaderVariant(const ShaderVariant&) = default;
	ShaderVariant& operator=(const ShaderVariant&) = default;
	ShaderVariant(ShaderVariant&&) = default;
	ShaderVariant& operator=(ShaderVariant&&) = default;
	~ShaderVariant() = default;

	inline bool IsUberShader() const;

	void AddFeatureSet(ShaderFeatureSet featureSet);
	void Build();
	
	void CleanBuild();
	void CleanAll();

	void SetCompiledProgram(StringCrc shaderFeaturesCrc, uint16_t programHandle);
	uint16_t GetCompiledProgram(StringCrc shaderFeaturesCrc) const;

	StringCrc GetFeaturesCrc(const ShaderFeatureSet& featureSet) const;
	inline bool IsFeaturesValid(StringCrc shaderFeaturesCrc) const;

	void SetShaderFeatureSets(std::vector<ShaderFeatureSet> sets);
	std::vector<ShaderFeatureSet>& GetShaderFeatureSets() { return m_featureSets; }
	const std::vector<ShaderFeatureSet>& GetShaderFeatureSets() const { return m_featureSets; }

	void SetFeatureCombines(std::vector<std::string> combines);
	std::vector<std::string>& GetFeatureCombines() { return m_featureCombines; }
	const std::vector<std::string>& GetFeatureCombines() const { return m_featureCombines; }

	void SetCompiledProgramHandles(std::map<uint32_t, uint16_t> handles);
	std::map<uint32_t, uint16_t>& GetCompiledProgramHandles() { return m_compiledProgramHandles; }
	const std::map<uint32_t, uint16_t>& GetCompiledProgramHandles() const { return m_compiledProgramHandles; }

private:
	// The ordering of this container ensures the stability when generating feature combination string.
	std::vector<ShaderFeatureSet> m_featureSets;
	// Parameters to compile shaders.
	std::vector<std::string> m_featureCombines;
	// Key: StringCrc(feature combine), Value: shader handle.
	std::map<uint32_t, uint16_t> m_compiledProgramHandles;

	bool m_isDirty = false;
};

}