#pragma once

#include "Core/Delegates/MulticastDelegate.hpp"

#include <cstdint>

struct SDL_Window;

namespace engine
{

class Window
{
public:
    struct Rect
    {
        int x, y, w, h;
    };

public:
    static void Init();
    static void Shutdown();
    static int GetDisplayMonitorCount();
    static const char* GetDisplayMonitorName(int index);
    static Rect GetDisplayMonitorMainRect(int index);
    static Rect GetDisplayMonitorWorkRect(int index);

public:
    Window() = delete;
    Window(const void* pParentHandle);
    Window(const char* pTitle, uint16_t width, uint16_t height);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;
    ~Window();
    
    void* GetHandle() const;
    //void* GetParentHandle() const;

    const char* GetTitle() const;
    void SetTitle(const char* pTitle);

    int GetWidth() const;
    int GetHeight() const;
    std::pair<int, int> GetSize() const;
    void SetSize(int w, int h);

    int GetPositionX() const;
    int GetPositionY() const;
    std::pair<int, int> GetPosition() const;
    void SetPosition(int x, int y);

    void Show();
    void Hide();
    void SetFullScreen(bool on);

    bool GetInputFocus() const;
    bool GetMouseFocus() const;
    void SetResizeable(bool on);
    void SetBordedLess(bool on);
    void SetWindowIcon(const char* pFilePath) const;
    void SetMouseVisible(bool isVisible, uint32_t x, uint32_t y);
    void WrapMouseInCenter() const;

    void Update();

    bool IsFocused() const { return m_isFocused; }

    bool ShouldClose() const { return m_isClosed; }
    void Close(bool bPushSdlEvent = true);
public:
    // Window
    Delegate<void(const char*)> OnDropFile;
    MulticastDelegate<void(uint16_t, uint16_t)> OnResize;

private:
    SDL_Window* m_pSDLWindow = nullptr;
    bool m_isClosed = false;
    bool m_isFocused = false;
};

}