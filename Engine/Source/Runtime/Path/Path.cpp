#include "Path.h"

#include "Log/Log.h"

namespace engine
{

namespace
{

constexpr size_t MAX_PATH_SIZE = 1024;

}

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
        // TODO : Need more test.
#if defined(_WIN32)
        return std::filesystem::path(value);
#elif defined(__linux__)
        return std::filesystem::path(value) / ".local" / "share";
#elif defined(__APPLE__)
        return std::filesystem::path(value) / "Library" / "Application Support";
#endif
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

} // namespace engine
