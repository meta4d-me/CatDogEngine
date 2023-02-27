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
	ResourceWatcher(std::string path)
	{
		m_watchPath = cd::MoveTemp(path);
		for (auto& file : std::filesystem::recursive_directory_iterator(m_watchPath))
		{
			m_modifyTimeCache[file.path().string()] = std::filesystem::last_write_time(file);
		}
	}

	ResourceWatcher() = default;
	ResourceWatcher(const ResourceWatcher&) = default;
	ResourceWatcher& operator=(const ResourceWatcher&) = default;
	ResourceWatcher(ResourceWatcher&&) = default;
	ResourceWatcher& operator=(ResourceWatcher&&) = default;
	~ResourceWatcher() = default;

private:
	std::string m_watchPath;
	std::unordered_map<std::string, std::filesystem::file_time_type> m_modifyTimeCache;
};


} // namespace editor 
