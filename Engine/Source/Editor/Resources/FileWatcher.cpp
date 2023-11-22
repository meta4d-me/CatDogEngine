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

class CallbackWrapper final
{
public:
    CallbackWrapper() = default;
    CallbackWrapper(const CallbackWrapper&) = delete;
    CallbackWrapper& operator=(const CallbackWrapper&) = delete;
    CallbackWrapper(CallbackWrapper&&) = delete;
    CallbackWrapper& operator=(CallbackWrapper&&) = delete;
    ~CallbackWrapper() = default;

    static void Callback(
        WatchID watchID, WatchAction action,
        const char* rootDir, const char* filePath,
        const char* oldFilePath, void* userData)
    {
        FileWatcher* pFileWatcher = static_cast<FileWatcher*>(userData);

        switch (action)
        {
            case DMON_ACTION_CREATE:
            {
                CD_TRACE("New file created");
                CD_TRACE("    Path : {0}", rootDir);
                CD_TRACE("    Name : {0}", filePath);

                // TODO : Dont assert when delegate dosent bind.
                pFileWatcher->GetWatchInfo(watchID.id).m_onCreate.Invoke(rootDir, filePath);

                break;
            }

            case DMON_ACTION_DELETE:
            {
                CD_TRACE("File deleted");
                CD_TRACE("    Path : {0}", rootDir);
                CD_TRACE("    Name : {0}", filePath);
                
                pFileWatcher->GetWatchInfo(watchID.id).m_onDelete.Invoke(rootDir, filePath);

                break;
            }

            case DMON_ACTION_MODIFY:
            {
                CD_TRACE("File Modified");
                CD_TRACE("    Path : {0}", rootDir);
                CD_TRACE("    Name : {0}", filePath);

                pFileWatcher->GetWatchInfo(watchID.id).m_onModify.Invoke(rootDir, filePath);

                break;
            }

            case DMON_ACTION_MOVE:
            {
                CD_TRACE("File moved");
                CD_TRACE("    Path : {0}", rootDir);
                CD_TRACE("    Old Name : {0}", oldFilePath);
                CD_TRACE("    New Name : {0}", filePath);
                
                pFileWatcher->GetWatchInfo(watchID.id).m_onMove.Invoke(rootDir, filePath, oldFilePath);

                break;
            }

            default:
            {
                CD_ERROR("Unknown WatchAction!");
            }
        }
    }
};

} // namespace

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
    m_fileWatchInfos.clear();
}

uint32_t FileWatcher::Watch(FileWatchInfo info)
{
    CD_INFO("Start watching {0}{1}", info.m_watchPath, (info.m_isrecursive ? " and its subpaths." : ""));

    WatchID watchID = dmon_watch(info.m_watchPath.c_str(), CallbackWrapper::Callback, static_cast<uint32_t>(info.m_isrecursive), this);
    m_fileWatchInfos[watchID.id] = cd::MoveTemp(info);

    return watchID.id;
}

void FileWatcher::UnWatch(uint32_t watchID)
{
    dmon_unwatch(WatchID{ watchID });
    m_fileWatchInfos.erase(watchID);
}

void FileWatcher::SetWatchInfos(std::map<uint32_t, FileWatchInfo> witchInfos)
{
    m_fileWatchInfos = cd::MoveTemp(witchInfos);
}

const FileWatchInfo& FileWatcher::GetWatchInfo(uint32_t id) const
{
    return m_fileWatchInfos.at(id);
}

const std::string& FileWatcher::GetWatchingPath(uint32_t id) const
{
    return m_fileWatchInfos.at(id).m_watchPath;
}

}