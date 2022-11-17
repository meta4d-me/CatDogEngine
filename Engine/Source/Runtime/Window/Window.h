#pragma once

#include <inttypes.h>

#include "Core/Delegates/MulticastDelegate.hpp"

struct SDL_Window;

namespace engine
{

class Window
{
public:
    Window() = delete;
    Window(const char* pTitle, uint16_t width, uint16_t height);
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;
    ~Window();

    void Update();
    void* GetNativeWindow() const { return m_pNativeWindowHandle; }
    bool ShouldClose();
    void Closed(bool bPushSdlEvent = true);
    void SetSize(uint16_t p_width, uint16_t p_height);
	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }

public:
    // Mouse
    MulticastDelegate<void()> OnMouseLBDown;
    MulticastDelegate<void()> OnMouseLBUp;
    MulticastDelegate<void()> OnMouseRBDown;
    MulticastDelegate<void()> OnMouseRBUp;
    MulticastDelegate<void()> OnMouseMBDown;
    MulticastDelegate<void()> OnMouseMBUp;
    MulticastDelegate<void(float)> OnMouseWheel;
    MulticastDelegate<void(int32_t, int32_t)> OnMouseMove;
    MulticastDelegate<void(int32_t, int32_t)> OnMouseMoveRelative;

    // Keyboard
    MulticastDelegate<void(int32_t, uint16_t)> OnKeyDown;
    MulticastDelegate<void(int32_t, uint16_t)> OnKeyUp;

    // Window
    MulticastDelegate<void(const char*)> OnDropFile;
    MulticastDelegate<void(uint16_t, uint16_t)> OnResize;

private:
    void* m_pNativeWindowHandle = nullptr;
    SDL_Window* m_pSDLWindow = nullptr;

    uint16_t m_width = 1;
    uint16_t m_height = 1;
    bool m_IsClosed = false;
};

}