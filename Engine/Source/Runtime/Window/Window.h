#pragma once

#include "Core/Delegates/MulticastDelegate.hpp"

#include <cstdint>

struct SDL_Window;

namespace engine
{

class Window
{
public:
    static void Init();
    static void Shutdown();

public:
    Window() = delete;
    Window(const char* pTitle, uint16_t width, uint16_t height, bool useFullScreen = false);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;
    ~Window();
    
    void* GetNativeHandle() const { return m_pNativeWindowHandle; }
    
	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }

    void SetTitle(const char* pTitle);
    void SetFullScreen(bool flag);
    void SetResizeable(bool flag);
    void SetBordedLess(bool flag);
    void SetSize(uint16_t width, uint16_t height);
    void SetWindowIcon(const char* pFilePath) const;

    void Update();

    bool ShouldClose() const { return m_isClosed; }
    void Close(bool bPushSdlEvent = true);
     
public:
    // Window
    Delegate<void(const char*)> OnDropFile;
    MulticastDelegate<void(uint16_t, uint16_t)> OnResize;

private:
    void* m_pNativeWindowHandle = nullptr;
    SDL_Window* m_pSDLWindow = nullptr;

    uint16_t m_width = 1;
    uint16_t m_height = 1;
    bool m_isClosed = false;
};

}