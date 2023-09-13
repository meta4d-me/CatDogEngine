#include "ShaderVariant.h"

#include "Base/Template.h"
#include "Log/Log.h"

#include <cassert>

namespace engine
{

bool ShaderVariant::IsUberShader() const
{
	assert((m_featureSets.empty() && m_featureCombines.empty()) || 
		(!m_featureSets.empty() && !m_featureCombines.empty()));
	return (!m_featureSets.empty());
}

void ShaderVariant::AddFeatureSet(ShaderFeatureSet featureSet)
{
	for (const auto& existingFeatureSet : m_featureSets)
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
	m_featureSets.emplace_back(cd::MoveTemp(featureSet));
}

void ShaderVariant::Build()
{
	if (!m_isDirty)
	{
		CD_ENGINE_TRACE("Shader features have no changes since the last build.");
		return;
	}

	CleanBuild();
	m_isDirty = false;

	for (const auto& featureSet : m_featureSets)
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

void ShaderVariant::CleanBuild()
{
	m_featureCombines.clear();
	m_compiledProgramHandles.clear();
	m_isDirty = true;
}

void ShaderVariant::CleanAll()
{
	CleanBuild();
	m_isDirty = false;

	m_featureSets.clear();
}

void ShaderVariant::SetCompiledProgram(StringCrc shaderFeaturesCrc, uint16_t programHandle)
{
	assert(IsFeaturesValid(shaderFeaturesCrc));
	m_compiledProgramHandles[shaderFeaturesCrc.Value()] = programHandle;
}

uint16_t ShaderVariant::GetCompiledProgram(StringCrc shaderFeaturesCrc) const
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

StringCrc ShaderVariant::GetFeaturesCrc(const ShaderFeatureSet& featureSet) const
{
	if (m_featureSets.empty() || featureSet.empty())
	{
		return DefaultUberShaderCrc;
	}

	std::stringstream ss;
	// Use the option order in m_shaderFeatureSets to ensure that inputs in different orders can get the same optionsCrc.
	for (const auto& registeredSet : m_featureSets)
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

bool ShaderVariant::IsFeaturesValid(StringCrc shaderFeaturesCrc) const
{
	return m_compiledProgramHandles.find(shaderFeaturesCrc.Value()) != m_compiledProgramHandles.end();
}

void ShaderVariant::SetShaderFeatureSets(std::vector<ShaderFeatureSet> sets)
{
	m_featureSets = cd::MoveTemp(sets);
}

void ShaderVariant::SetFeatureCombines(std::vector<std::string> combines)
{
	m_featureCombines = cd::MoveTemp(combines);
}

void ShaderVariant::SetCompiledProgramHandles(std::map<uint32_t, uint16_t> handles)
{
	m_compiledProgramHandles = cd::MoveTemp(handles);
}

}