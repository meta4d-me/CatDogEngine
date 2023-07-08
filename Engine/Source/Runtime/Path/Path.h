#pragma once 

#include "Graphics/GraphicsBackend.h"

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>

namespace engine
{

class Path
{
public:
	static constexpr size_t MAX_PATH_SIZE = 1024;
	static constexpr const char* EngineName = "CatDogEngine";
	static constexpr const char* ShaderInputExtension = ".sc";
	static constexpr const char* ShaderOutputExtension = ".bin";

	static std::optional<std::filesystem::path> GetApplicationDataPath();

	static std::filesystem::path GetEngineBuiltinShaderPath();
	static std::filesystem::path GetEngineResourcesPath();
	static std::filesystem::path GetEditorResourcesPath();
	static std::filesystem::path GetProjectsSharedPath();

	static engine::GraphicsBackend GetGraphicsBackend();
	static void SetGraphicsBackend(engine::GraphicsBackend backend);

	static std::string GetBuiltinShaderInputPath(const char* pShaderName);
	static std::filesystem::path GetShaderOutputDirectory();
	static std::string GetShaderOutputPath(const char* pInputFilePath, const std::string& options = "");
	static std::string GetTextureOutputFilePath(const char* pInputFilePath, const char* extension);
	static std::string GetTerrainTextureOutputFilePath(const char* pInputFilePath, const char* extension);

private:
	static const char* GetPlatformPathKey();
	static std::filesystem::path GetPlatformAppDataPath(char(&value)[MAX_PATH_SIZE]);

private:
	static engine::GraphicsBackend s_backend;
};

}