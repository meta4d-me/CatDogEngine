#include "ShaderSchema.h"

#include "Base/Template.h"
#include "Log/Log.h"

#include <algorithm>
#include <cassert>
#include <sstream>

namespace engine
{

namespace details
{

constexpr const char* UberNames[] =
{
	"", // Use empty string to represent default shader option in the name so we can reuse non-uber built shader.
	"ALBEDO;",
	"NORMAL_MAP;",
	"USE_ORM;",
	"EMISSIVE;",
	"IBL;",
	"AREAL_LIGHT;",
};

static_assert(static_cast<int>(Uber::COUNT) == sizeof(UberNames) / sizeof(char*),
	"Uber and names mismatch.");

constexpr const char* GetUberName(Uber uber)
{
	return UberNames[static_cast<size_t>(uber)];
}

constexpr const char* LoadingStatusName[] =
{
	"MISSING_RESOURCES",
	"LOADING_SHADERS",
	"LOADING_TEXTURES",
	"LOADING_ERROR",
};

static_assert(static_cast<int>(LoadingStatus::COUNT) == sizeof(LoadingStatusName) / sizeof(char*),
	"LoadingStatus and names mismatch.");

CD_FORCEINLINE const char* GetLoadingStatusName(LoadingStatus status)
{
	return LoadingStatusName[static_cast<size_t>(status)];
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

void ShaderSchema::AddSingleUberOption(LoadingStatus status, std::string path)
{
	if (m_loadingStatusFSPath.contains(status))
	{
		CD_ENGINE_WARN("Single uber opertion {0} has already been added!", details::GetLoadingStatusName(status));
		return;
	}
	m_loadingStatusFSPath[status] = cd::MoveTemp(path);

	std::string loadingStatusName = details::GetLoadingStatusName(status);
	assert(!IsUberOptionValid(StringCrc(loadingStatusName)));
	m_compiledProgramHandles[StringCrc(loadingStatusName).Value()] = InvalidProgramHandle;
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

StringCrc ShaderSchema::GetProgramCrc(const std::vector<Uber>& options) const
{
	if (options.empty())
	{
		return DefaultUberOption;
	}

	// Ignore option which contain in parameter but not contain in m_uberOptions.
	std::stringstream ss;
	for (const auto& registered : m_uberOptions)
	{
		if (std::find(options.begin(), options.end(), registered) != options.end())
		{
			ss << details::GetUberName(registered);
		}
	}
	return StringCrc(ss.str());
}

StringCrc ShaderSchema::GetProgramCrc(const LoadingStatus& status) const
{
	return StringCrc(details::GetLoadingStatusName(status));
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