#pragma once 

#include <cstdlib>
#include <filesystem>
#include <optional>

namespace engine
{

class Path
{
public:
	static constexpr size_t MAX_PATH_SIZE = 1024;

public:
	static constexpr const char* EngineName = "CatDogEngine";

	static std::optional<std::filesystem::path> GetApplicationDataPath();

	static std::filesystem::path GetEngineBuiltinShaderPath()
	{
		return std::filesystem::path(CDENGINE_BUILTIN_SHADER_PATH);
	}
	static std::filesystem::path GetEngineResourcesPath()
	{
		return std::filesystem::path(CDENGINE_RESOURCES_ROOT_PATH);
	}
	static std::filesystem::path GetEditorResourcesPath()
	{
		return std::filesystem::path(CDEDITOR_RESOURCES_ROOT_PATH);
	}

private:
	static const char* GetPlatformPathKey();
	static std::filesystem::path GetPlatformAppDataPath(char(&value)[MAX_PATH_SIZE]);
};

}