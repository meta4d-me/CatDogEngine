#pragma once

#include <functional>
#include <string>
#include <thread>

namespace editor
{

class FileWatcher
{
private:
    typedef std::function<void(std::string)> FileWatcherCallback;

public:
    FileWatcher(std::string path);
    ~FileWatcher();

    FileWatcher() = delete;
    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;
    FileWatcher(FileWatcher&&) = delete;
    FileWatcher& operator=(FileWatcher&&) = delete;

    void SetFileAddedCallback(FileWatcherCallback callback);
    void SetFileModifiedCallback(FileWatcherCallback callback);
    void SetFileRemovedCallback(FileWatcherCallback callback);
    void SetFileRenamedOldCallback(FileWatcherCallback callback);
    void SetFileRenamedNewCallback(FileWatcherCallback callback);

    void Start();
    void Stop();

    void SetPath(std::string path);
    std::string& GetPath() { return m_path; }
    const std::string& GetPath() const { return m_path; }

    const bool GetIsRunning() const { return m_isRunning; }

    void EnableWatchSubTree() { m_isWatchSubTree = true; }
    void DisableWatchSubTree() { m_isWatchSubTree = false; }
    const bool GetIsWatchSubTree() const { return m_isWatchSubTree; }

private:
    FileWatcherCallback m_fileAddedCallback;
    FileWatcherCallback m_fileModifiedCallback;
    FileWatcherCallback m_fileRemovedCallback;
    FileWatcherCallback m_fileRenamedOldCallBack;
    FileWatcherCallback m_fileRenamedNewCallBack;

    std::string m_path;
    bool m_isRunning;
    bool m_isWatchSubTree;

    std::thread m_thread;

    void Watch();
};

}