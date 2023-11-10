#pragma once

#include "Base/Template.h"
#include "IModule.h"

#include <memory>
#include <string>

namespace engine
{

enum class ModuleStatus
{
	Unload,
	NotFound,
	Loaded,
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
	const char* GetName() const { return m_name.c_str(); }

	void SetFilePath(std::string path) { m_filePath = cd::MoveTemp(path); }
	const char* GetFilePath() const { return m_filePath.c_str(); }

	void SetStatus(ModuleStatus status) { m_status = status; }
	ModuleStatus GetStatus() const { return m_status; }

private:
	std::string m_name;
	std::string m_filePath;

	ModuleStatus m_status = ModuleStatus::Unload;
	bool m_autoLoad = false;
	
	void* m_handle = nullptr;
	std::unique_ptr<IModule> m_module;
};

}