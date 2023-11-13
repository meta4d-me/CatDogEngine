#pragma once

#include <cstdint>
#include <functional>

#include "Log/Log.h"
#include "Path/Path.h"
#include "Rendering/RenderContext.h"
#include "Window/Window.h"

#define DMON_LOG_DEBUG(s) do { CD_INFO(s); } while(0)
#define DMON_LOG_ERROR(s) do { CD_ERROR(s); assert(false); } while(0)

#define DMON_IMPL
#include "dmon.h"

namespace editor
{

using WatchID = dmon_watch_id;
using WatchAction = dmon_action;

class FileWatcher final
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
        WatchID watchID = dmon_watch(rootDir, callback, flags, userData);

        CD_INFO("Start watching {0}", rootDir);
        CD_INFO("    Watch ID {0}", watchID.id);

        m_witchInfos[watchID.id] = rootDir;

        return watchID;
    }

    void UnWatch(WatchID watchID)
    {
        dmon_unwatch(watchID);
    }

    std::map<uint32_t, std::string> m_witchInfos;
};

class FileWatchCallbackWrapper final
{
public:
    FileWatchCallbackWrapper() = default;
    FileWatchCallbackWrapper(const FileWatchCallbackWrapper&) = delete;
    FileWatchCallbackWrapper& operator=(const FileWatchCallbackWrapper&) = delete;
    FileWatchCallbackWrapper(FileWatchCallbackWrapper&&) = delete;
    FileWatchCallbackWrapper& operator=(FileWatchCallbackWrapper&&) = delete;
    ~FileWatchCallbackWrapper() = default;

    static void Callback(
        WatchID id, WatchAction action,
        const char* rootDir, const char* filePath,
        const char* oldFilePath, void* user)
    {
        switch (action)
        {
            case DMON_ACTION_CREATE:
                CD_TRACE("New file created");
                CD_TRACE("    Path : {0}", rootDir);
                CD_TRACE("    Name : {0}", filePath);
                break;

            case DMON_ACTION_DELETE:
                CD_TRACE("File deleted");
                CD_TRACE("    Path : {0}", rootDir);
                CD_TRACE("    Name : {0}", filePath);
                break;

            case DMON_ACTION_MODIFY:
                CD_TRACE("File Modified");
                CD_TRACE("    Path : {0}", rootDir);
                CD_TRACE("    Name : {0}", filePath);

                if (m_pWindow->GetInputFocus() && engine::Path::GetExtension(filePath) != ".sc")
                {
                    return;
                }

                m_pRenderContext->CheckModifiedProgram(engine::Path::GetFileNameWithoutExtension(filePath));

                break;

            case DMON_ACTION_MOVE:
                CD_TRACE("File moved");
                CD_TRACE("    Path : {0}", rootDir);
                CD_TRACE("    Old Name : {0}", oldFilePath);
                CD_TRACE("    New Name : {0}", filePath);
                break;

            default:
                CD_WARN("Unknown WatchAction!");
        }
    }

    static void SetRenderContext(engine::RenderContext* pRenderContext)
    {
        m_pRenderContext = pRenderContext;
    }

    static void SetWindow(engine::Window* pWindow)
    {
        m_pWindow = pWindow;
    }

    static engine::RenderContext* m_pRenderContext;
    static engine::Window* m_pWindow;
};

engine::RenderContext* FileWatchCallbackWrapper::m_pRenderContext = nullptr;
engine::Window* FileWatchCallbackWrapper::m_pWindow = nullptr;

}