#pragma once

#include "Core/Delegates/Delegate.hpp"
#include "Rendering/ShaderType.h"
#include "Scene/MaterialTextureType.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <queue>
#include <span>
#include <string>
#include <unordered_map>

namespace editor
{

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

struct TaskOutputCallbacks
{
	engine::Delegate<void(uint32_t handle, std::span<const char> str)> onOutput;
	engine::Delegate<void(uint32_t handle, std::span<const char> str)> onErrorOutput;
};

using TaskHandle = uint32_t;

// ResourceBuilder is used to create processes to build different resource types.
// So it is OK to update in the main thread or work thread.
// For resource build tasks which are using dll calls, it will be wrapped as a task to multithreading JobSystem.
class ResourceBuilder final
{
public:
	static constexpr uint32_t MaxTaskCount = 64;
	static constexpr uint32_t InvalidHandle = MaxTaskCount;

#define INVALID_TASK_HANDLE { InvalidHandle }

	static constexpr uint8_t SkipStatus =
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

	TaskHandle AddTask(Process* process);
	TaskHandle AddShaderBuildTask(engine::ShaderType shaderType, const char* pInputFilePath, const char* pOutputFilePath, const char* pShaderFeatures = "", TaskOutputCallbacks callbacks = {});
	TaskHandle AddIrradianceCubeMapBuildTask(const char* pInputFilePath, const char* pOutputFilePath, TaskOutputCallbacks callbacks = {});
	TaskHandle AddRadianceCubeMapBuildTask(const char* pInputFilePath, const char* pOutputFilePath, TaskOutputCallbacks callbacks = {});
	TaskHandle AddTextureBuildTask(cd::MaterialTextureType textureType, const char* pInputFilePath, const char* pOutputFilePath, TaskOutputCallbacks callbacks = {});

	void Update(bool doPrintLog = false, bool doPrintErrorLog = true);
	uint32_t GetCurrentTaskCount() const;
	bool IsIdle() const;

private:
	ResourceBuilder();
	~ResourceBuilder();

	void ReadModifyCacheFile();
	void WriteModifyCacheFile();

	bool HasNewModifyTimeCache() const;
	void UpdateModifyTimeCache();
	void ClearModifyTimeCache();
	void DeleteModifyTimeCache();

	std::string GetModifyCacheFilePath();

	ProcessStatus CheckFileStatus(const char* pInputFilePath, const char* pOutputFilePath);

private:
	uint32_t m_numActiveTask;
	std::array<TaskHandle, MaxTaskCount> m_handleList;
	std::array<Process*, MaxTaskCount> m_tasks;
	std::queue<TaskHandle> m_taskQueue;

	std::unordered_map<std::string, long long> m_modifyTimeCache;
	// We always access to fragment shader multiple times by using ubre options.
	// So we can not update fragment shader's modify time every time we found it has been modified.
	std::unordered_map<std::string, long long> m_newModifyTimeCache;
};

}