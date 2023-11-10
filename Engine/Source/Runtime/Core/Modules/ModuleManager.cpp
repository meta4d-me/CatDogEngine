#include "ModuleManager.h"

#include "Module.h"
#include "Path/Path.h"

namespace engine
{

Module* ModuleManager::AddModule(const char* pFilePath)
{
	std::string moduleName = Path::GetFileNameWithoutExtension(pFilePath);
	StringCrc moduleCrc(moduleName);
	if (auto* pModule = GetModule(moduleCrc))
	{
		return pModule;
	}

	auto module = std::make_unique<Module>();
	Module* pModule = module.get();
	m_allModules[moduleCrc] = cd::MoveTemp(module);

	pModule->SetName(cd::MoveTemp(moduleName));
	pModule->SetFilePath(pFilePath);
	pModule->SetStatus(Path::FileExists(pFilePath) ? ModuleStatus::Unload : ModuleStatus::NotFound);
	return pModule;
}

bool ModuleManager::FindModule(StringCrc moduleCrc) const
{
	return m_allModules.find(moduleCrc) != m_allModules.end();
}

Module* ModuleManager::GetModule(StringCrc moduleCrc) const
{
	auto itModule = m_allModules.find(moduleCrc);
	return itModule != m_allModules.end() ? itModule->second.get() : nullptr;
}

void ModuleManager::RemoveModule(StringCrc moduleCrc)
{
	if (auto itModule = m_allModules.find(moduleCrc); itModule != m_allModules.end())
	{
		m_allModules.erase(itModule);
	}
}

}