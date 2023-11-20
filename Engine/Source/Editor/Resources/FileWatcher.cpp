#include "FileWatcher.h"

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

namespace
{

class ShaderHotModifyCallbackWrapper final
{
public:
    ShaderHotModifyCallbackWrapper() = default;
    ShaderHotModifyCallbackWrapper(const ShaderHotModifyCallbackWrapper&) = delete;
    ShaderHotModifyCallbackWrapper& operator=(const ShaderHotModifyCallbackWrapper&) = delete;
    ShaderHotModifyCallbackWrapper(ShaderHotModifyCallbackWrapper&&) = delete;
    ShaderHotModifyCallbackWrapper& operator=(ShaderHotModifyCallbackWrapper&&) = delete;
    ~ShaderHotModifyCallbackWrapper() = default;

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

                if (m_pWindow->GetInputFocus() || engine::Path::GetExtension(filePath) != ".sc")
                {
                    // Return when window get focus.
                    // Return when a non-shader file is detected.
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

    static engine::RenderContext* m_pRenderContext;
    static engine::Window* m_pWindow;
};

engine::RenderContext* ShaderHotModifyCallbackWrapper::m_pRenderContext = nullptr;
engine::Window* ShaderHotModifyCallbackWrapper::m_pWindow = nullptr;

}

FileWatcher::FileWatcher()
{
    Init();
}

FileWatcher::~FileWatcher()
{
    Deinit();
}

void FileWatcher::Init()
{
    dmon_init();
}

void FileWatcher::Deinit()
{
    dmon_deinit();
    m_watchInfos.clear();
}

uint32_t FileWatcher::WatchShaders(const char* rootDir)
{
    ShaderHotModifyCallbackWrapper::m_pRenderContext = m_pRenderContext;
    ShaderHotModifyCallbackWrapper::m_pWindow = m_pWindow;

    WatchID watchID = dmon_watch(rootDir, ShaderHotModifyCallbackWrapper::Callback, 0, nullptr);
    
    CD_INFO("Start watching {0}", rootDir);
    CD_INFO("    Watch ID {0}", watchID.id);
    
    m_watchInfos[watchID.id] = rootDir;

    return watchID.id;
}

void FileWatcher::UnWatch(uint32_t watchID)
{
    dmon_unwatch(WatchID{ watchID });
    m_watchInfos.erase(watchID);
}

void FileWatcher::SetWatchInfos(std::map<uint32_t, const char*> witchInfos)
{
    m_watchInfos = cd::MoveTemp(witchInfos);
}

const char* FileWatcher::GetWatchingPath(uint32_t id) const
{
    return m_watchInfos.at(id);
}

}