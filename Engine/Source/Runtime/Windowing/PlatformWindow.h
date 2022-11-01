#pragma once

#include <inttypes.h>

#include "Core/Delegates/Delegate.hpp"

struct SDL_Window;

namespace engine
{

class PlatformWindow
{
public:
    PlatformWindow() = delete;
    PlatformWindow(const char* pTitle, uint16_t width, uint16_t height);
    PlatformWindow(const PlatformWindow&) = delete;
    PlatformWindow& operator=(const PlatformWindow&) = delete;
    PlatformWindow(PlatformWindow&&) = delete;
    PlatformWindow& operator=(PlatformWindow&&) = delete;
    ~PlatformWindow();

    void Update();
    void* GetNativeWindow() const { return m_pNativeWindowHandle; }
    bool ShouldClose();
    void Closed(bool bPushSdlEvent = true);
    void SetSize(uint16_t p_width, uint16_t p_height);
	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }

public:
    Delegate<void(const uint8_t, const uint8_t)> OnMouseDown;
    Delegate<void(const uint8_t, const uint8_t)> OnMouseUp;
    Delegate<void(const int32_t, const int32_t)> OnMouseMotion;
    Delegate<void(const int32_t, const uint16_t)> OnKeyDown;
    Delegate<void(const int32_t, const uint16_t)> OnKeyUp;

private:
    void* m_pNativeWindowHandle = nullptr;
    SDL_Window* m_pSDLWindow = nullptr;

    uint16_t m_width = 1;
    uint16_t m_height = 1;
    bool m_IsClosed = false;
};

}