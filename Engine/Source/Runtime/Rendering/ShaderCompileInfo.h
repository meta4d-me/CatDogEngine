#pragma once

#include "Base/Template.h"

#include <string>

namespace engine
{

class ShaderCompileInfo
{
public:
	ShaderCompileInfo(std::string name, std::string combine = "") :
		m_programName(cd::MoveTemp(name)), m_featuresCombine(cd::MoveTemp(combine)) {}
	ShaderCompileInfo() = default;
	ShaderCompileInfo(const ShaderCompileInfo&) = default;
	ShaderCompileInfo& operator=(const ShaderCompileInfo&) = default;
	ShaderCompileInfo(ShaderCompileInfo&&) = default;
	ShaderCompileInfo& operator=(ShaderCompileInfo&&) = default;
	~ShaderCompileInfo() = default;

	std::string m_programName;
	std::string m_featuresCombine;

	bool operator==(const ShaderCompileInfo& other) const
	{
		return (m_programName == other.m_programName) && (m_featuresCombine == other.m_featuresCombine);
	}

	bool operator<(const ShaderCompileInfo& other) const
	{
		if (m_programName < other.m_programName)
		{
			return true;
		}
		if (m_programName > other.m_programName)
		{
			return false;
		}

		return m_featuresCombine < other.m_featuresCombine;
	}
};

}