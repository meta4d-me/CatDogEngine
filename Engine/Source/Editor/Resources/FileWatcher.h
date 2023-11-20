#pragma once

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
    void SetRenderContext(engine::RenderContext* pRenderContext) { m_pRenderContext = pRenderContext; }
    void SetWindow(engine::Window* pWindow) { m_pWindow = pWindow; }
    void Deinit();

    uint32_t WatchShaders(const char* rootDir);
    void UnWatch(uint32_t watchID);

    void SetWatchInfos(std::map<uint32_t, const char*>);
    std::map<uint32_t, const char*>& GetWatchInfos() { return m_watchInfos; }
    const std::map<uint32_t, const char*>& GetWitchInfos() const { return m_watchInfos; }
    const char* GetWatchingPath(uint32_t id) const;

private:
    std::map<uint32_t, const char*> m_watchInfos;

    engine::RenderContext* m_pRenderContext;
    engine::Window* m_pWindow;
};

}