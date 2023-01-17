#pragma once

#include "Process/Process.h"
#include "Scene/MaterialTextureType.h"

#include <queue>

namespace editor
{

class Process;

// ResourceBuilder is used to create processes to build different resource types.
// So it is OK to update in the main thread or work thread.
// For resource build tasks which are using dll calls, it will be wrapped as a task to multithreading JobSystem.
class ResourceBuilder final
{
public:
	ResourceBuilder(const ResourceBuilder&) = delete;
	ResourceBuilder& operator=(const ResourceBuilder&) = delete;
	ResourceBuilder(ResourceBuilder&&) = delete;
	ResourceBuilder& operator=(ResourceBuilder&&) = delete;
	~ResourceBuilder() = default;

	static ResourceBuilder& Get()
	{
		static ResourceBuilder s_instance;
		return s_instance;
	}

	void AddTask(Process process);
	void AddCubeMapBuildTask(const char* pInputFilePath, const char* pOutputFileName);
	void AddShaderBuildTask(const char* pInputFilePath, const char* pOutputFilePath);
	void AddTextureBuildTask(const char* pInputFilePath, const char* pOutputFilePath, cd::MaterialTextureType textureType);
	void Update();

private:
	ResourceBuilder() = default;

	std::queue<Process> m_buildTasks;
};

}