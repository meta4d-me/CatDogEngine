#include "ShaderSchema.h"

#include "Base/Template.h"

#include <algorithm>
#include <cassert>
#include <sstream>

namespace engine
{

namespace Utils
{

constexpr const char* UberName[] =
{
	"DEFAULT;",
	"IBL;",
	"NORMAL_MAP;",
	"OCCLUSION;",
};

static_assert(static_cast<int>(Uber::Count) == sizeof(UberName) / sizeof(char*),
	"Uber and names mismatch.");

CD_FORCEINLINE const char* GetUberName(Uber buer)
{
	return UberName[static_cast<size_t>(buer)];
}

} // namespace Utils


ShaderSchema::ShaderSchema(std::string vsPath, std::string fsPath)
{
	m_vertexShaderPath = cd::MoveTemp(vsPath);
	m_fragmentShaderPath = cd::MoveTemp(fsPath);

	// Always register DEFAULT at the begining.
	m_uberCombines.emplace_back(Utils::GetUberName(Uber::DEFAULT));
	m_compiledProgramHandles[StringCrc(Utils::GetUberName(Uber::DEFAULT)).Value()] = InvalidProgramHandle;
	m_uberOptions.emplace_back(Uber::DEFAULT);
}

void ShaderSchema::RegisterUberOption(Uber uberOption)
{
	if (std::find(m_uberOptions.begin(), m_uberOptions.end(), uberOption) != m_uberOptions.end())
	{
		CD_ENGINE_WARN("Uber option {0} already has been registered!", Utils::GetUberName(uberOption));
		return;
	}

	std::string newOption = Utils::GetUberName(uberOption);
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

void ShaderSchema::AddSingleUberOption(std::string uberOption)
{
	assert(!IsUberOptionValid(StringCrc(uberOption)));
	m_compiledProgramHandles[StringCrc(uberOption).Value()] = InvalidProgramHandle;
}

bool ShaderSchema::IsUberOptionValid(StringCrc uberOption) const
{
	return m_compiledProgramHandles.contains(uberOption.Value());
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

StringCrc ShaderSchema::GetOptionsCombination(const std::initializer_list<Uber>& options) const
{
	std::stringstream ss;
	for (const auto& registered : m_uberOptions)
	{
		if (std::find(options.begin(), options.end(), registered) != options.end())
		{
			ss << Utils::GetUberName(registered);
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
	if (m_uberOptionToFSBlobs.contains(uberOption.Value()))
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