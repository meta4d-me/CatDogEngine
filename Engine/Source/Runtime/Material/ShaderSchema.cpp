#include "ShaderSchema.h"

#include "Base/Template.h"
#include "Log/Log.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <unordered_set>

namespace engine
{

namespace details
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

} // namespace Detail


ShaderSchema::ShaderSchema(std::string vsPath, std::string fsPath)
{
	m_vertexShaderPath = cd::MoveTemp(vsPath);
	m_fragmentShaderPath = cd::MoveTemp(fsPath);

	// Always register DEFAULT at the begining.
	m_uberCombines.emplace_back(details::GetUberName(Uber::DEFAULT));
	m_compiledProgramHandles[StringCrc(details::GetUberName(Uber::DEFAULT)).Value()] = InvalidProgramHandle;
	m_uberOptions.emplace_back(Uber::DEFAULT);
}

void ShaderSchema::RegisterUberOption(Uber uberOption)
{
	if (std::find(m_uberOptions.begin(), m_uberOptions.end(), uberOption) != m_uberOptions.end())
	{
		CD_ENGINE_WARN("Uber option {0} already has been registered!", details::GetUberName(uberOption));
		return;
	}

	std::string newOption = details::GetUberName(uberOption);
	std::vector<std::string> newOptions = { newOption };
	
	assert(!m_uberCombines.empty());
	for (auto it = m_uberCombines.begin() + 1; it != m_uberCombines.end(); ++it)
	{
		// m_uberCombines[0] will always be DEFAULT,
		// which is unnecessary to combine with other options.
		newOptions.push_back({ *it + newOption });
	}
	m_uberCombines.insert(m_uberCombines.end(), newOptions.begin(), newOptions.end());

	for (const auto &newOpt : newOptions)
	{
		assert(!IsUberOptionValid(StringCrc(newOpt)));
		m_compiledProgramHandles[StringCrc(newOpt).Value()] = InvalidProgramHandle;
	}

	m_uberOptions.emplace_back(cd::MoveTemp(uberOption));
}

bool ShaderSchema::IsUberOptionValid(StringCrc uberOption) const
{
	return m_compiledProgramHandles.find(uberOption.Value()) != m_compiledProgramHandles.end();
}

void ShaderSchema::SetCompiledProgram(StringCrc uberOption, uint16_t programHandle)
{
	assert(IsUberOptionValid(uberOption));
	m_compiledProgramHandles[uberOption.Value()] = programHandle;
}

uint16_t ShaderSchema::GetCompiledProgram(StringCrc uberOption) const
{
	auto itProgram = m_compiledProgramHandles.find(uberOption.Value());
	assert(itProgram != m_compiledProgramHandles.end());
	uint16_t programHandle = itProgram->second;

	// Registered but not compiled.
	assert(programHandle != InvalidProgramHandle);

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
			ss << details::GetUberName(registered);
		}
	}
	return StringCrc(ss.str());
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