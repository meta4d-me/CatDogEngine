#include "Path.h"

#include "Log/Log.h"

namespace engine
{

std::optional<std::filesystem::path> Path::GetApplicationDataPath()
{

    char value[MAX_PATH_SIZE];
    size_t len;
    errno_t err = getenv_s(&len, value, MAX_PATH_SIZE, GetPlatformPathKey());
    if (err)
    {
        CD_ENGINE_ERROR(err);
        return std::nullopt;
    }
    else
    {
        return GetPlatformAppDataPath(value);
    }
}

const char* Path::GetPlatformPathKey()
{
#if defined(_WIN32)
    return "LOCALAPPDATA";
#elif defined(__linux__)
    return "HOME";
#elif defined(__APPLE__)
    return "HOME";
#else
    #error Unsupport platform!
#endif
}

std::filesystem::path Path::GetPlatformAppDataPath(char(&value)[MAX_PATH_SIZE])
{
    // TODO : Need more test.
#if defined(_WIN32)
    return std::filesystem::path(value);
#elif defined(__linux__)
    return std::filesystem::path(value) / ".local" / "share";
#elif defined(__APPLE__)
    return std::filesystem::path(value) / "Library" / "Application Support";
#else
    #error Unsupport platform!
#endif
}

std::filesystem::path Path::GetEngineBuiltinShaderPath()
{
    return std::filesystem::path(CDENGINE_BUILTIN_SHADER_PATH);
}

std::filesystem::path Path::GetEngineResourcesPath()
{
    return std::filesystem::path(CDENGINE_RESOURCES_ROOT_PATH);
}

std::filesystem::path Path::GetEditorResourcesPath()
{
    return std::filesystem::path(CDEDITOR_RESOURCES_ROOT_PATH);
}

std::string Path::GetBuiltinShaderInputPath(const char* pShaderName)
{
    return (GetEngineBuiltinShaderPath() / std::filesystem::path(pShaderName).stem()).replace_extension(ShaderInputExtension).string();
}

std::string Path::GetShaderOutputPath(const char* pInputFilePath, const std::string& options)
{
    std::string outputShaderFileName = std::filesystem::path(pInputFilePath).stem().string();

    if (!options.empty())
    {
        std::string appendName = "_" + options;
        std::replace(appendName.begin(), appendName.end(), ';', '_');
        outputShaderFileName += appendName;

        assert(!outputShaderFileName.empty());
        const auto last = outputShaderFileName.end() - 1;
        if (*last == '_')
        {
            outputShaderFileName.erase(last);
        }
    }

    return ((GetEngineResourcesPath() / "Shaders" / cd::MoveTemp(outputShaderFileName)).replace_extension(ShaderOutputExtension)).string();
}

std::string Path::GetTextureOutputFilePath(const char* pInputFilePath, const char* extension)
{
    return ((GetEngineResourcesPath() / "Textures" / std::filesystem::path(pInputFilePath).stem()).replace_extension(extension)).string();
}

std::string Path::GetTerrainTextureOutputFilePath(const char* pInputFilePath, const char* extension)
{
    return ((GetEngineResourcesPath() / "Textures" / "Terrain" / std::filesystem::path(pInputFilePath).stem()).replace_extension(extension)).string();
}

}