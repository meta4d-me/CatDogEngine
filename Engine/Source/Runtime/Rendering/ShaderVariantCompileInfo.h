#pragma once

#include <string>

namespace engine
{

class ShaderVariantCompileInfo
{
public:
	std::string m_programName;
	std::string m_featuresCombine;

	bool operator==(const ShaderVariantCompileInfo& other) const
	{
		return (m_programName == other.m_programName) && (m_featuresCombine == other.m_featuresCombine);
	}
};

}