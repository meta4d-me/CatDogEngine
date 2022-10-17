#pragma once

#include <inttypes.h>

struct SDL_Window;

namespace engine
{

class FirstPersonCameraController;

class PlatformWindow
{
public:
    PlatformWindow(const char* pTitle, uint16_t width, uint16_t height, FirstPersonCameraController* pCameraController);
    ~PlatformWindow();

    void Update();
    void* GetNativeWindow() const;
    bool ShouldClose();
    void Closed(bool bPushSdlEvent = true);
    void SetSize(uint16_t p_width, uint16_t p_height);
	uint16_t GetWidth() const { return m_Width; }
	uint16_t GetHeight() const { return m_Height; }

private:
    void* m_Ndt = nullptr;
    void* m_Nwh = nullptr;

    uint16_t m_Width = 1;
    uint16_t m_Height = 1;
    uint16_t m_LastWidth = 1;
    uint16_t m_LastHeight = 1;

    bool m_IsClosed = false;
    SDL_Window* m_Window = nullptr;
    FirstPersonCameraController* m_CameraController = nullptr;
};

}