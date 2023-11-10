#pragma once

#include "Core/StringCrc.h"

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
	~ModuleManager() = default;

	Module* AddModule(const char* pFilePath);
	bool FindModule(StringCrc moduleCrc) const;
	Module* GetModule(StringCrc moduleCrc) const;
	void RemoveModule(StringCrc moduleCrc);

private:
	std::map<StringCrc, std::unique_ptr<Module>> m_allModules;
};

}