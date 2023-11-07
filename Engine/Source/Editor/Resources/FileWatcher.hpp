#pragma once

#include <cstdint>
#include <functional>

#include "Log/Log.h"

#define DMON_LOG_DEBUG(s) do { CD_INFO(s); } while(0)
#define DMON_LOG_ERROR(s) do { CD_ERROR(s); assert(false); } while(0)

#define DMON_IMPL
#include "dmon.h"

namespace editor
{

using WatchID = dmon_watch_id;
using WatchAction = dmon_action;

class FileWatcher
{

public:
    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;
    FileWatcher(FileWatcher&&) = delete;
    FileWatcher& operator=(FileWatcher&&) = delete;

    FileWatcher()
    {
        Init();
    }

    ~FileWatcher()
    {
        Deinit();
    }

    void Init()
    {
        dmon_init();
    }

    void Deinit()
    {
        dmon_deinit();
    }

    WatchID Watch(
        const char* rootDir,
        void (*callback)(
            WatchID watchID, WatchAction action,
            const char* dirName, const char* filename,
            const char* oldName, void* user),
        uint32_t flags, void* userData)
    {
        return dmon_watch(rootDir, callback, flags, userData);
    }

    void UnWatch(WatchID watchID)
    {
        dmon_unwatch(watchID);
    }
};

}