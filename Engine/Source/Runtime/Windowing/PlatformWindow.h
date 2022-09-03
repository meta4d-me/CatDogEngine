#pragma once

#include <inttypes.h>

struct SDL_Window;

namespace engine
{

struct WindowCreateDescriptor
{
    char* title;
    uint16_t width;
    uint16_t height;
};

class PlatformWindow
{
public:
    PlatformWindow(const WindowCreateDescriptor& createDescriptor);
    ~PlatformWindow();

    void Update();
    void* GetNativeWindow() const;
    bool ShouldClose();
    void Closed(bool bPushSdlEvent = true);
    void SetSize(uint16_t p_width, uint16_t p_height);
	uint16_t GetWidth() const { return m_Width; }
	uint16_t GetHeight() const { return m_Height; }

private:
    void* m_Ndt;
    void* m_Nwh;
    uint16_t m_Width;
    uint16_t m_Height;
    uint16_t m_LastWidth;
    uint16_t m_LastHeight;
    bool m_IsClosed;
    SDL_Window* m_Window;
};

}