#include "ShaderSchema.h"

#include "Base/Template.h"
#include "Log/Log.h"

#include <algorithm>
#include <cassert>
#include <sstream>

namespace engine
{

ShaderSchema::ShaderSchema(std::string progeamName, std::string vsPath, std::string fsPath)
{
	m_programName = cd::MoveTemp(progeamName);
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
	m_shaderFeatureSets.insert(cd::MoveTemp(featureSet));
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

	// Should ShaderSchema handle uber shader without shader feature?
	m_allFeatureCombines.insert("");
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

std::string ShaderSchema::GetFeaturesCombine(const ShaderFeatureSet& featureSet) const
{
	if (m_shaderFeatureSets.empty() || featureSet.empty())
	{
		return "";
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
	return ss.str();
}

StringCrc ShaderSchema::GetFeaturesCombineCrc(const ShaderFeatureSet& featureSet) const
{
	if (m_shaderFeatureSets.empty() || featureSet.empty())
	{
		return DefaultUberShaderCrc;
	}

	return StringCrc(GetFeaturesCombine(featureSet));
}

}