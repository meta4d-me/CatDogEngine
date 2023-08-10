#include "ShaderSchema.h"

#include "Base/Template.h"
#include "Log/Log.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <string>

namespace engine
{

namespace
{

constexpr const char* UberNames[] =
{
	"", // Use empty string to represent default shader option in the name so we can reuse non-uber built shader.
	"ALBEDOMAP;",
	"NORMALMAP;",
	"ORMMAP;",
	"EMISSIVEMAP;",
	"IBL;",
	"ATM",
	"AREAL_LIGHT;",
};

static_assert(static_cast<int>(Uber::COUNT) == sizeof(UberNames) / sizeof(char*),
	"Uber and names mismatch.");

constexpr const char* GetUberName(Uber uber)
{
	return UberNames[static_cast<size_t>(uber)];
}

}

ShaderSchema::ShaderSchema(std::string vsPath, std::string fsPath)
{
	m_vertexShaderPath = cd::MoveTemp(vsPath);
	m_fragmentShaderPath = cd::MoveTemp(fsPath);

	m_isDirty = false;
}

void ShaderSchema::AddUberOption(Uber uberOption)
{
	if (std::find(m_uberOptions.begin(), m_uberOptions.end(), uberOption) != m_uberOptions.end())
	{
		CD_ENGINE_TRACE("Uber option {0} already has been registered!", GetUberName(uberOption));
		return;
	}
	
	m_isDirty = true;
	m_uberOptions.emplace_back(uberOption);
}

void ShaderSchema::SetConflictOptions(Uber a, Uber b)
{
	const auto& range = m_conflictOptions.equal_range(GetUberName(a));
	for (auto conflict = range.first; conflict != range.second; ++conflict)
	{
		if (conflict->second == GetUberName(b))
		{
			CD_ENGINE_TRACE("Conflict uber option combine ({0}, {1}) are already exist.", GetUberName(a), GetUberName(b));
			return;
		}
	}

	m_isDirty = true;
	m_conflictOptions.emplace(GetUberName(a), GetUberName(b));
	m_conflictOptions.emplace(GetUberName(b), GetUberName(a));
}

void ShaderSchema::Build()
{
	if (!m_isDirty)
	{
		CD_ENGINE_TRACE("Uber shader options have no changes since the last build.");
		return;
	}

	CleanBuild();

	for(auto itOption = m_uberOptions.begin(); itOption != m_uberOptions.end(); ++itOption)
	{
		std::string newOption = GetUberName(*itOption);
		std::vector<std::string> newOptions = { newOption };
		const auto& conflictRange = m_conflictOptions.equal_range(newOption);

		for(const auto& cobine : m_uberCombines)
		{
			bool isConflict = false;
			for (auto conflict = conflictRange.first; conflict != conflictRange.second; ++conflict)
			{
				if (cobine.find(conflict->second) != std::string::npos)
				{
					// Skip conflict uber option combine.
					isConflict = true;
					break;
				}
			}
			if (!isConflict)
			{
				newOptions.emplace_back(cobine + newOption);
			}
		}
		m_uberCombines.insert(m_uberCombines.end(), newOptions.begin(), newOptions.end());

		for (const auto& newOpt : newOptions)
		{
			assert(!IsUberOptionsValid(StringCrc(newOpt)));
			m_compiledProgramHandles[StringCrc(newOpt).Value()] = InvalidProgramHandle;
		}
	}
}

void ShaderSchema::CleanBuild()
{
	m_uberCombines.clear();
	m_compiledProgramHandles.clear();
	m_isDirty = false;
}

void ShaderSchema::CleanAll()
{
	CleanBuild();

	m_uberOptions.clear();
	m_conflictOptions.clear();
}

void ShaderSchema::SetCompiledProgram(StringCrc uberOption, uint16_t programHandle)
{
	assert(IsUberOptionsValid(uberOption));
	m_compiledProgramHandles[uberOption.Value()] = programHandle;
}

uint16_t ShaderSchema::GetCompiledProgram(StringCrc uberOption) const
{
	auto itProgram = m_compiledProgramHandles.find(uberOption.Value());

	if (itProgram == m_compiledProgramHandles.end())
	{
		CD_ENGINE_ERROR("Unregistered uber shader options!");
		return InvalidProgramHandle;
	}

	uint16_t programHandle = itProgram->second;

	if (programHandle == InvalidProgramHandle)
	{
		CD_ENGINE_ERROR("Uncompiled uber shader options！");
		return InvalidProgramHandle;
	}

	return programHandle;
}

StringCrc ShaderSchema::GetOptionsCrc(const std::unordered_set<Uber>& options) const
{
	if (options.empty())
	{
		return DefaultUberShaderCrc;
	}

	std::stringstream ss;
	// Use the option order in m_uberOptions to ensure that inputs in different orders can get the same optionsCrc.
	for (const auto& registered : m_uberOptions)
	{
		// Ignore option which contain in parameter but not contain in m_uberOptions.
		if (options.find(registered) != options.end())
		{
			ss << GetUberName(registered);
		}
	}
	return StringCrc(ss.str());
}

bool ShaderSchema::IsUberOptionsValid(StringCrc uberOption) const
{
	return m_compiledProgramHandles.find(uberOption.Value()) != m_compiledProgramHandles.end();
}

void ShaderSchema::AddUberOptionVSBlob(ShaderBlob shaderBlob)
{
	if (m_pVSBlob)
	{
		// TODO : process vertex uber shaders.
		return;
	}

	m_pVSBlob = std::make_unique<ShaderBlob>(cd::MoveTemp(shaderBlob));
}

void ShaderSchema::AddUberOptionFSBlob(StringCrc uberOption, ShaderBlob shaderBlob)
{
	if (m_uberOptionToFSBlobs.find(uberOption.Value()) != m_uberOptionToFSBlobs.end())
	{
		return;
	}

	m_uberOptionToFSBlobs[uberOption.Value()] = std::make_unique<ShaderBlob>(cd::MoveTemp(shaderBlob));
}

const ShaderSchema::ShaderBlob& ShaderSchema::GetFSBlob(StringCrc uberOption) const
{
	auto itBlob = m_uberOptionToFSBlobs.find(uberOption.Value());
	assert(itBlob != m_uberOptionToFSBlobs.end());
	return *(itBlob->second.get());
}

}