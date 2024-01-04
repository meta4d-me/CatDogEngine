#pragma once

#include "Base/Template.h"
#include "ECWorld/Entity.h"

#include <set>
#include <string>

namespace engine
{

class ShaderCompileInfo
{
public:
	ShaderCompileInfo(Entity entity, std::string name, std::string combine = "") :
		m_entity(entity), m_programName(cd::MoveTemp(name)), m_featuresCombine(cd::MoveTemp(combine)) {}
	ShaderCompileInfo() = delete;
	ShaderCompileInfo(const ShaderCompileInfo&) = default;
	ShaderCompileInfo& operator=(const ShaderCompileInfo&) = default;
	ShaderCompileInfo(ShaderCompileInfo&&) = default;
	ShaderCompileInfo& operator=(ShaderCompileInfo&&) = default;
	~ShaderCompileInfo() = default;

	Entity m_entity = INVALID_ENTITY;
	std::string m_programName;
	std::string m_featuresCombine;
	std::set<uint32_t> m_taskHandles;

	bool operator==(const ShaderCompileInfo& other) const
	{
		return (m_entity == other.m_entity) && (m_programName == other.m_programName) && (m_featuresCombine == other.m_featuresCombine);
	}

	bool operator<(const ShaderCompileInfo& other) const
	{
		if (m_entity < other.m_entity)
		{
			return true;
		}
		if (m_entity > other.m_entity)
		{
			return false;
		}

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