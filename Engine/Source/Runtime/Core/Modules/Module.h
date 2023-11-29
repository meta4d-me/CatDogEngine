#pragma once

#include "Base/Template.h"
#include "IModule.h"
#include "IO/InputArchive.hpp"
#include "IO/OutputArchive.hpp"

#include <memory>
#include <string>

namespace engine
{

enum class ModuleStatus
{
	Unload,
	Loaded,
};

enum class ModuleLoadResult
{
	AlreadyLoaded,
	FileNotExist,
	LoadDllFailure,
	InterfaceMissing,
	InitFailure,
	Success
};

class Module
{
public:
	Module() = default;
	Module(const Module&) = delete;
	Module& operator=(const Module&) = delete;
	Module(Module&&) = default;
	Module& operator=(Module&&) = default;
	~Module() = default;

	void SetName(std::string name) { m_name = cd::MoveTemp(name); }
	std::string& GetName() { return m_name; }
	const std::string& GetName() const { return m_name; }

	void SetFilePath(std::string path) { m_filePath = cd::MoveTemp(path); }
	std::string& GetFilePath() { return m_filePath; }
	const std::string& GetFilePath() const { return m_filePath; }

	void SetAutoLoad(bool on) { m_autoLoad = on; }
	bool& GetAutoLoad() { return m_autoLoad; }
	bool GetAutoLoad() const { return m_autoLoad; }

	bool IsLoaded() const { return ModuleStatus::Loaded == m_status; }
	ModuleLoadResult Load();
	void Unload();

	template<bool SwapBytesOrder>
	Module& operator<<(cd::TInputArchive<SwapBytesOrder>& inputArchive)
	{
		inputArchive >> GetName() >> GetFilePath() >> GetAutoLoad();
		return *this;
	}

	template<bool SwapBytesOrder>
	const Module& operator>>(cd::TOutputArchive<SwapBytesOrder>& outputArchive) const
	{
		outputArchive << GetName() << GetFilePath() << GetAutoLoad();
		return *this;
	}

private:
	// Configs
	std::string m_name;
	std::string m_filePath;
	bool m_autoLoad = false;

	// Runtime
	ModuleStatus m_status = ModuleStatus::Unload;
	void* m_handle = nullptr;
	std::unique_ptr<IModule> m_module;
};

}