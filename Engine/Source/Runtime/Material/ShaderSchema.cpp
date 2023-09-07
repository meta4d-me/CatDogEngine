#include "ShaderSchema.h"

#include "Base/Template.h"
#include "Log/Log.h"

#include <algorithm>
#include <cassert>
#include <sstream>

namespace engine
{

namespace
{

constexpr const char* ShaderFeatureNames[] =
{
	"", // Use empty string to represent default shader option in the name so we can reuse non-uber built shader.
	"ALBEDOMAP;",
	"NORMALMAP;",
	"ORMMAP;",
	"EMISSIVEMAP;",
	"IBL;",
	"ATM;",
	"AREALLIGHT;",
};

static_assert(static_cast<int>(ShaderFeature::COUNT) == sizeof(ShaderFeatureNames) / sizeof(char*),
	"Shader features and names mismatch.");

CD_FORCEINLINE constexpr const char* GetFeatureName(ShaderFeature feature)
{
	return ShaderFeatureNames[static_cast<size_t>(feature)];
}

}

ShaderSchema::ShaderSchema(std::string vsPath, std::string fsPath)
{
	m_vertexShaderPath = cd::MoveTemp(vsPath);
	m_fragmentShaderPath = cd::MoveTemp(fsPath);

	m_isDirty = false;
}

void ShaderSchema::AddFeatureSet(ShaderFeatureSet featureSet)
{
	for (const auto& existingFeatureSet : m_shaderFeatureSets)
	{
		for (const auto& newFeature : featureSet)
		{
			if (existingFeatureSet.find(newFeature) != existingFeatureSet.end())
			{
				CD_ENGINE_WARN("Shader feature {0} repetitive, skip current feature set adding!", GetFeatureName(newFeature));
				return;
			}
		}
	}

	m_isDirty = true;
	m_shaderFeatureSets.emplace_back(cd::MoveTemp(featureSet));
}

void ShaderSchema::Build()
{
	if (!m_isDirty)
	{
		CD_ENGINE_TRACE("Shader features have no changes since the last build.");
		return;
	}

	CleanBuild();
	m_isDirty = false;

	for (const auto& featureSet : m_shaderFeatureSets)
	{
		for (const auto& feature : featureSet)
		{
			std::string newFeatureName = GetFeatureName(feature);
			std::vector<std::string> newFeatureCombines = { newFeatureName };

			for (const auto& combine : m_featureCombines)
			{
				// Skip combination which has features in same set.
				bool skip = false;
				for (const auto& conflict : featureSet)
				{
					if (conflict == feature)
					{
						continue;
					}
					if (combine.find(GetFeatureName(conflict)) != std::string::npos)
					{
						skip = true;
						break;
					}
				}
				if (!skip)
				{
					newFeatureCombines.emplace_back(combine + newFeatureName);
				}
			}
			m_featureCombines.insert(m_featureCombines.end(), newFeatureCombines.begin(), newFeatureCombines.end());
			for (const auto& combine : newFeatureCombines)
			{
				assert(!IsFeaturesValid(StringCrc(combine)));
				m_compiledProgramHandles[StringCrc(combine).Value()] = InvalidProgramHandle;
			}
		}
	}

	// ShaderSchema also handle non-uber case.
	m_featureCombines.emplace_back("");
	m_compiledProgramHandles[DefaultUberShaderCrc.Value()] = InvalidProgramHandle;
}

void ShaderSchema::CleanBuild()
{
	m_featureCombines.clear();
	m_compiledProgramHandles.clear();
	m_isDirty = true;
}

void ShaderSchema::CleanAll()
{
	CleanBuild();
	m_isDirty = false;

	m_shaderFeatureSets.clear();
}

void ShaderSchema::SetCompiledProgram(StringCrc shaderFeaturesCrc, uint16_t programHandle)
{
	assert(IsFeaturesValid(shaderFeaturesCrc));
	m_compiledProgramHandles[shaderFeaturesCrc.Value()] = programHandle;
}

uint16_t ShaderSchema::GetCompiledProgram(StringCrc shaderFeaturesCrc) const
{
	auto itProgram = m_compiledProgramHandles.find(shaderFeaturesCrc.Value());

	if (itProgram == m_compiledProgramHandles.end())
	{
		CD_ENGINE_ERROR("Unregistered shader features!");
		return InvalidProgramHandle;
	}

	uint16_t programHandle = itProgram->second;

	if (programHandle == InvalidProgramHandle)
	{
		CD_ENGINE_ERROR("Uncompiled shader features");
		return InvalidProgramHandle;
	}

	return programHandle;
}

StringCrc ShaderSchema::GetFeaturesCrc(const ShaderFeatureSet& featureSet) const
{
	if (featureSet.empty())
	{
		return DefaultUberShaderCrc;
	}

	std::stringstream ss;
	// Use the option order in m_shaderFeatureSets to ensure that inputs in different orders can get the same optionsCrc.
	for (const auto& registeredSet : m_shaderFeatureSets)
	{
		// Ignore option which contain in parameter but not contain in m_shaderFeatureSets.
		for (const auto& registeredFeature : registeredSet)
		{
			if (featureSet.find(registeredFeature) != featureSet.end())
			{
				ss << GetFeatureName(registeredFeature);
			}
		}

	}
	return StringCrc(ss.str());
}

bool ShaderSchema::IsFeaturesValid(StringCrc shaderFeaturesCrc) const
{
	return m_compiledProgramHandles.find(shaderFeaturesCrc.Value()) != m_compiledProgramHandles.end();
}

void ShaderSchema::AddUberVSBlob(ShaderBlob shaderBlob)
{
	if (m_pVSBlob)
	{
		// TODO : process vertex uber shaders.
		return;
	}

	m_pVSBlob = std::make_unique<ShaderBlob>(cd::MoveTemp(shaderBlob));
}

void ShaderSchema::AddUberFSBlob(StringCrc shaderFeaturesCrc, ShaderBlob shaderBlob)
{
	if (m_shaderFeaturesToFSBlobs.find(shaderFeaturesCrc.Value()) != m_shaderFeaturesToFSBlobs.end())
	{
		return;
	}

	m_shaderFeaturesToFSBlobs[shaderFeaturesCrc.Value()] = std::make_unique<ShaderBlob>(cd::MoveTemp(shaderBlob));
}

const ShaderSchema::ShaderBlob& ShaderSchema::GetFSBlob(StringCrc shaderFeaturesCrc) const
{
	auto itBlob = m_shaderFeaturesToFSBlobs.find(shaderFeaturesCrc.Value());
	assert(itBlob != m_shaderFeaturesToFSBlobs.end());
	return *(itBlob->second.get());
}

}