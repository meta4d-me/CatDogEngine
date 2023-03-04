#pragma once

#include "Base/Template.h"

#include <chrono>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>

namespace editor
{

enum class FileStatus
{
	Created,
	Modified,
	Erased,
};

class ResourceWatcher final
{
public:
    ResourceWatcher(std::string path);

    ResourceWatcher() = delete;
    ResourceWatcher(const ResourceWatcher&) = default;
    ResourceWatcher& operator=(const ResourceWatcher&) = default;
    ResourceWatcher(ResourceWatcher&&) = default;
    ResourceWatcher& operator=(ResourceWatcher&&) = default;
    ~ResourceWatcher() = default;

    void Check(const std::function<void(std::string, FileStatus)>& Action);

private:
	std::string m_watchPath;
	std::unordered_map<std::string, std::filesystem::file_time_type> m_modifyTimeCache;
};

} // namespace editor 
