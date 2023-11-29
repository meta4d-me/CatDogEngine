#pragma once

#include "Core/StringCrc.h"
#include "IO/InputArchive.hpp"
#include "IO/OutputArchive.hpp"

#include <map>
#include <memory>
#include <string>

namespace engine
{

class Module;

class ModuleManager
{
public:
	ModuleManager() = default;
	ModuleManager(const ModuleManager&) = delete;
	ModuleManager& operator=(const ModuleManager&) = delete;
	ModuleManager(ModuleManager&&) = default;
	ModuleManager& operator=(ModuleManager&&) = default;
	~ModuleManager();

	void LoadModules(bool checkAutoLoad = true);
	void UnloadModules();

	Module* AddModule(const char* pFilePath);
	bool FindModule(StringCrc moduleCrc) const;
	Module* GetModule(StringCrc moduleCrc) const;
	void RemoveModule(StringCrc moduleCrc);

	template<bool SwapBytesOrder>
	ModuleManager& operator<<(cd::TInputArchive<SwapBytesOrder>& inputArchive)
	{
		uint32_t moduleCount;
		inputArchive >> moduleCount;
		for (uint32_t moduleIndex = 0U; moduleIndex < moduleCount; ++moduleIndex)
		{
			Module module;
			inputArchive >> module;
			AddModule(module.GetFilePath().c_str());
		}

		return *this;
	}

	template<bool SwapBytesOrder>
	const ModuleManager& operator>>(cd::TOutputArchive<SwapBytesOrder>& outputArchive) const
	{
		outputArchive << static_cast<uint32_t>(m_allModules.size());
		for (const auto& module : m_allModules)
		{
			outputArchive << module;
		}

		return *this;
	}

private:
	std::map<StringCrc, std::unique_ptr<Module>> m_allModules;
};

}