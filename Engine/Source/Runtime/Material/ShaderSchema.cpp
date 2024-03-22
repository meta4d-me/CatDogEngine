#include "ShaderSchema.h"

#include "Log/Log.h"

#include <algorithm>
#include <cassert>
#include <sstream>

namespace engine
{

void ShaderSchema::Build()
{
	if (!m_isDirty)
	{
		CD_ENGINE_TRACE("Shader features have no changes since the last build.");
		return;
	}

	CleanBuild();

	for (const auto& featureSet : m_shaderFeatureSets)
	{
		for (const auto& feature : featureSet)
		{
			std::string newFeatureName = GetFeatureName(feature);
			std::set<std::string> newFeatureCombines = { newFeatureName };

			for (const auto& combine : m_allFeatureCombines)
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
					newFeatureCombines.insert(combine + newFeatureName);
				}
			}
			m_allFeatureCombines.insert(newFeatureCombines.begin(), newFeatureCombines.end());
		}
	}

	// ShaderScheme also handles case without ShaderFeature.
	m_allFeatureCombines.insert("");
	m_isDirty = false;
}

void ShaderSchema::CleanBuild()
{
	m_allFeatureCombines.clear();
	m_isDirty = true;
}

void ShaderSchema::CleanAll()
{
	CleanBuild();
	m_isDirty = false;

	m_shaderFeatureSets.clear();
}

void ShaderSchema::AddFeatureSet(ShaderFeatureSet featureSet)
{
	// We trate shader features as set to handel mutually exclusive keywords.
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
	m_shaderFeatureSets.insert(cd::MoveTemp(featureSet));
}

std::optional<ShaderFeatureSet> ShaderSchema::GetConflictFeatureSet(const ShaderFeature feature) const
{
	for (const auto& shaderFeatureSet : m_shaderFeatureSets)
	{
		if (shaderFeatureSet.find(feature) != shaderFeatureSet.end())
		{
			return shaderFeatureSet;
		}
	}

	return std::nullopt;
}

std::string ShaderSchema::GetFeaturesCombine(const ShaderFeatureSet& featureSet) const
{
	if (m_shaderFeatureSets.empty() || featureSet.empty())
	{
		return "";
	}

	std::stringstream ss;
	// Use the Shader Feature order in m_shaderFeatureSets to ensure that inputs in different orders can get a same StringCrc.
	for (const auto& registeredSet : m_shaderFeatureSets)
	{
		// Ignore Shader Feature which contain in parameter but not contain in m_shaderFeatureSets.
		for (const auto& registeredFeature : registeredSet)
		{
			if (featureSet.find(registeredFeature) != featureSet.end())
			{
				// We assume that theres no conflicting Features in the incoming featureSet parameter.
				ss << GetFeatureName(registeredFeature);
				continue;
			}
		}
	}

	return ss.str();
}

std::set<std::string>& ShaderSchema::GetAllFeatureCombines()
{
	assert(!m_isDirty);
	return m_allFeatureCombines;
}

const std::set<std::string>& ShaderSchema::GetAllFeatureCombines() const
{
	assert(!m_isDirty);
	return m_allFeatureCombines;
}

}