#pragma once

#include "Process/Process.h"
#include "Scene/MaterialTextureType.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <queue>
#include <string>
#include <unordered_map>

namespace editor
{

enum class ShaderType
{
	None,
	Compute,
	Vertex,
	Fragment
};

enum class ProcessStatus : uint8_t
{
	None           = 1 << 0,
	InputNotExist  = 1 << 1,
	OutputNotExist = 1 << 2,
	InputModified  = 1 << 3,
	InputAdded     = 1 << 4,
	Stable         = 1 << 5,
};

class Process;

// ResourceBuilder is used to create processes to build different resource types.
// So it is OK to update in the main thread or work thread.
// For resource build tasks which are using dll calls, it will be wrapped as a task to multithreading JobSystem.
class ResourceBuilder final
{
public:
	static constexpr uint8_t s_SkipStatus =
		static_cast<uint8_t>(ProcessStatus::None) |
		static_cast<uint8_t>(ProcessStatus::InputNotExist) |
		static_cast<uint8_t>(ProcessStatus::Stable);

public:
	ResourceBuilder(const ResourceBuilder&) = delete;
	ResourceBuilder& operator=(const ResourceBuilder&) = delete;
	ResourceBuilder(ResourceBuilder&&) = delete;
	ResourceBuilder& operator=(ResourceBuilder&&) = delete;

	static ResourceBuilder& Get()
	{
		static ResourceBuilder s_instance;
		return s_instance;
	}

	bool AddTask(Process process);
	bool AddCubeMapBuildTask(const char* pInputFilePath, const char* pOutputFilePath);
	bool AddShaderBuildTask(ShaderType shaderType, const char* pInputFilePath, const char* pOutputFilePath, const char *pUberOptions = nullptr);
	bool AddTextureBuildTask(cd::MaterialTextureType textureType, const char* pInputFilePath, const char* pOutputFilePath);
	void Update();

	void UpdateModifyTimeCache();
	void ClearModifyTimeCache();
	void DeleteModifyTimeCache();

private:
	ResourceBuilder();
	~ResourceBuilder();

	void ReadModifyCacheFile();
	void WriteModifyCacheFile();

	std::string GetModifyCacheFilePath();

	ProcessStatus CheckFileStatus(const char* pInputFilePath, const char* pOutputFilePath);

	std::queue<Process> m_buildTasks;
	
	std::unordered_map<std::string, long long> m_modifyTimeCache;
	// We always access to fragment ahsder multiple times by using ubre options.
	// So we can not update fragment shader's modify time every time we found it has been modified.
	std::unordered_map<std::string, long long> m_newModifyTimeCache;
};

}