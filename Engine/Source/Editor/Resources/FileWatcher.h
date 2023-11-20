#pragma once

#include "Resources/FileWatchInfo.h"

#include <cstdint>
#include <map>

namespace engine
{

class RenderContext;
class Window;

}

namespace editor
{

class FileWatcher final
{

public:
    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;
    FileWatcher(FileWatcher&&) = delete;
    FileWatcher& operator=(FileWatcher&&) = delete;

    FileWatcher();
    ~FileWatcher();

    void Init();
    void Deinit();

    uint32_t Watch(FileWatchInfo info);
    void UnWatch(uint32_t watchID);

    void SetWatchInfos(std::map<uint32_t, FileWatchInfo>);
    std::map<uint32_t, FileWatchInfo>& GetWatchInfos() { return m_fileWatchInfos; }
    const std::map<uint32_t, FileWatchInfo>& GetWatchInfos() const { return m_fileWatchInfos; }

    const FileWatchInfo& GetWatchInfo(uint32_t id) const;
    const std::string& GetWatchingPath(uint32_t id) const;

private:
    std::map<uint32_t, FileWatchInfo> m_fileWatchInfos;
};

}