#include "FileWatcher.h"

#include "Base/Template.h"
#include "Log/Log.h"

#include <filesystem>

#ifdef _WIN32
	#include <Windows.h>
#else
	// TODO : Remaining cross-platform code.
#endif

namespace editor
{

namespace
{

constexpr size_t BUFFER_SIZE = 1024;

}

FileWatcher::FileWatcher(std::string path)
    : m_fileAddedCallback(nullptr), m_fileModifiedCallback(nullptr), m_fileRemovedCallback(nullptr),
    m_fileRenamedOldCallBack(nullptr), m_fileRenamedNewCallBack(nullptr),
    m_path(cd::MoveTemp(path)), m_isRunning(false), m_isWatchSubTree(false) {}

FileWatcher::~FileWatcher()
{
    Stop();
}

void FileWatcher::SetFileAddedCallback(FileWatcherCallback callback)
{
    m_fileAddedCallback = callback;
}

void FileWatcher::SetFileModifiedCallback(FileWatcherCallback callback)
{
    m_fileModifiedCallback = callback;
}

void FileWatcher::SetFileRemovedCallback(FileWatcherCallback callback)
{
    m_fileRemovedCallback = callback;
}

void FileWatcher::SetFileRenamedOldCallback(FileWatcherCallback callback)
{
    m_fileRenamedOldCallBack = callback;
}

void FileWatcher::SetFileRenamedNewCallback(FileWatcherCallback callback)
{
    m_fileRenamedNewCallBack = callback;
}

void FileWatcher::Start()
{
    if (!std::filesystem::exists(m_path) || !std::filesystem::is_directory(m_path))
    {
        CD_ERROR("Unknown file watcher path!");
        return;
    }

    if (m_isRunning)
    {
        CD_ERROR("File watcher is already running!");
        return;
    }

    CD_INFO("Start watching \"{0}\", subtree {1}.", m_path, (m_isWatchSubTree ? "included" : "ignored"));

    m_isRunning = true;
    m_thread = std::thread(&FileWatcher::Watch, this);
}

void FileWatcher::Stop()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }
}

void FileWatcher::SetPath(std::string path)
{
    m_path = cd::MoveTemp(path);
}

void FileWatcher::Watch()
{
#ifdef _WIN32
    HANDLE pathHandle = CreateFileA(m_path.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    if (INVALID_HANDLE_VALUE == pathHandle)
    {
        CD_ERROR("FileWatcher create path handle failed with error code {0}", GetLastError());
        return;
    }

    char buffer[BUFFER_SIZE];
    DWORD bytesReturned = { 0 };
    OVERLAPPED overlapped = { 0 };
    overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);
    if (nullptr == overlapped.hEvent)
    {
        CD_ERROR("Filewatcher create event failed with error code {0}", GetLastError());
        CloseHandle(pathHandle);
        return;
    }

    while (m_isRunning)
    {
        if (!ResetEvent(overlapped.hEvent))
        {
            CD_ERROR("FileWatcher reset event failed with error code {0}", GetLastError());
            break;
        }

        constexpr uint32_t WatchStatus = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY;

        if (!ReadDirectoryChangesW(pathHandle, buffer, BUFFER_SIZE, m_isWatchSubTree, WatchStatus, &bytesReturned, &overlapped, nullptr))
        {
            CD_ERROR("FileWatcher read directory changes failed with error code {0}", GetLastError());
            break;
        }

        DWORD waitResault = WaitForSingleObject(overlapped.hEvent, 100);
        
        if (WAIT_TIMEOUT == waitResault)
        {
            continue;
        }
        else if (WAIT_OBJECT_0 != waitResault)
        {
            CD_ERROR("FileWatcher wait for IO failed with error code {0}", GetLastError());
            break;
        }

        DWORD overlappedReadByteCount;
        if (!GetOverlappedResult(pathHandle, &overlapped, &overlappedReadByteCount, false))
        {
            CD_ERROR("FileWatcher get overlapped result failed with error code {0}", GetLastError());
            break;
        }

        FILE_NOTIFY_INFORMATION* pFileNotifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

        while(true)
        {
            int fileNameSize = pFileNotifyInfo->FileNameLength / sizeof(WCHAR);
            int utf8Size = WideCharToMultiByte(CP_UTF8, 0, pFileNotifyInfo->FileName, fileNameSize, nullptr, 0, nullptr, nullptr);
            std::string fileName(utf8Size, 0);
            WideCharToMultiByte(CP_UTF8, 0, pFileNotifyInfo->FileName, fileNameSize, fileName.data(), utf8Size, nullptr, nullptr);
            std::string fullFilePath = (std::filesystem::path(m_path) / cd::MoveTemp(fileName)).generic_string();

            DWORD fileAction = pFileNotifyInfo->Action;

           switch (fileAction)
           {
               case FILE_ACTION_ADDED:
                   CD_INFO("File added {0}", fullFilePath);
                   if (m_fileAddedCallback)
                   {
                       m_fileAddedCallback(fullFilePath);
                   }
                   break;

               case FILE_ACTION_MODIFIED:
                   CD_INFO("File modified {0}", fullFilePath);
                   if (m_fileModifiedCallback)
                   {
                       m_fileModifiedCallback(fullFilePath);
                   }
                   break;

               case FILE_ACTION_REMOVED:
                   CD_INFO("File removed {0}", fullFilePath);
                   if (m_fileRemovedCallback)
                   {
                       m_fileRemovedCallback(fullFilePath);
                   }
                   break;

               case FILE_ACTION_RENAMED_OLD_NAME:
                   CD_INFO("File renamed old {0}", fullFilePath);
                   if (m_fileRenamedOldCallBack)
                   {
                       m_fileRenamedOldCallBack(fullFilePath);
                   }
                   break;

               case FILE_ACTION_RENAMED_NEW_NAME:
                   CD_INFO("File renamed new {0}", fullFilePath);
                   if (m_fileRenamedNewCallBack)
                   {
                       m_fileRenamedNewCallBack(fullFilePath);
                   }
                   break;

               default:
                   CD_ERROR("{0} unknown file whatcher status!", fullFilePath);
                   break;
           }

           if (0 == pFileNotifyInfo->NextEntryOffset)
           {
               break;
           }
           
           pFileNotifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(pFileNotifyInfo) + pFileNotifyInfo->NextEntryOffset);
        }

        memset(buffer, 0, bytesReturned);
    }

    CloseHandle(pathHandle);
    CloseHandle(overlapped.hEvent);

#else
    // TODO : Remaining cross-platform code.
#endif

}

}