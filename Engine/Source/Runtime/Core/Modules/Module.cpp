#include "Module.h"

#include "Core/OS/DllUtils.h"
#include "Path/Path.h"

#include <cassert>

namespace engine
{

ModuleLoadResult Module::Load()
{
	if (IsLoaded())
	{
		assert(m_handle && m_module);
		return ModuleLoadResult::AlreadyLoaded;
	}

	bool dllFileExists = Path::FileExists(m_filePath.c_str());
	if (!dllFileExists)
	{
		return ModuleLoadResult::FileNotExist;
	}

	m_handle = DllUtils::LoadDll(m_filePath.c_str());
	if (!m_handle)
	{
		return ModuleLoadResult::LoadDllFailure;
	}

	auto initFunc = reinterpret_cast<ModuleInitFunctionPtr>(DllUtils::LoadDllFunction(m_handle, ModuleInitFunctioName));
	if (!initFunc)
	{
		Unload();
		return ModuleLoadResult::InterfaceMissing;
	}

	void* pModule = initFunc();
	if (!pModule)
	{
		Unload();
		return ModuleLoadResult::InitFailure;
	}

	m_module.reset(reinterpret_cast<engine::IModule*>(pModule));
	m_status = ModuleStatus::Loaded;

	return ModuleLoadResult::Success;
}

void Module::Unload()
{
	if (m_module)
	{
		m_module->Shutdown();
		m_module.reset(nullptr);
	}

	if (m_handle)
	{
		DllUtils::UnloadDll(m_handle);
		m_handle = nullptr;
	}

	m_status = ModuleStatus::Unload;
}

}