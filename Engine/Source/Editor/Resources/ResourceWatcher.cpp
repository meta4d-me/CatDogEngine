#include "ResourceWatcher.h"

namespace editor
{

ResourceWatcher::ResourceWatcher(std::string path)
{
	m_watchPath = cd::MoveTemp(path);
	for (auto& file : std::filesystem::recursive_directory_iterator(m_watchPath))
	{
		m_modifyTimeCache[file.path().string()] = std::filesystem::last_write_time(file);
	}
}

void ResourceWatcher::Check(const std::function<void(std::string, FileStatus)>& Action)
{
    // Check if a file was erased.
    auto it = m_modifyTimeCache.begin();
    while (it != m_modifyTimeCache.end())
    {
        if (!std::filesystem::exists(it->first))
        {
            Action(it->first, FileStatus::Erased);
            it = m_modifyTimeCache.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Check if a file was created or modified.
    for (const auto& file : std::filesystem::recursive_directory_iterator(m_watchPath))
    {
        auto lastTime = std::filesystem::last_write_time(file);

        if (!m_modifyTimeCache.contains(file.path().string()))
        {
            // File creation.
            m_modifyTimeCache[file.path().string()] = lastTime;
            Action(file.path().string(), FileStatus::Created);
        }
        else if (m_modifyTimeCache[file.path().string()] != lastTime)
        {
            // File modification.
            m_modifyTimeCache[file.path().string()] = lastTime;
            Action(file.path().string(), FileStatus::Modified);
        }
        // Do nothing if file already exist and has no modification.
    }
}

} // namespace editor
