#include "Window.h"

#include "Input.h"

#include <SDL.h>
#include <SDL_syswm.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <string>

namespace engine
{

void Window::Init()
{
	// JoyStick : SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER
	SDL_Init(SDL_INIT_EVENTS);
	SDL_SetHintWithPriority("SDL_BORDERLESS_RESIZABLE_STYLE", "1", SDL_HINT_OVERRIDE);
	SDL_SetHintWithPriority("SDL_BORDERLESS_WINDOWED_STYLE", "1", SDL_HINT_OVERRIDE);
}

void Window::Shutdown()
{
	SDL_Quit();
}

int Window::GetDisplayMonitorCount()
{
	return SDL_GetNumVideoDisplays();
}

const char* Window::GetDisplayMonitorName(int index)
{
	return SDL_GetDisplayName(index);
}

Window::Rect Window::GetDisplayMonitorMainRect(int index)
{
	SDL_Rect rect;
	SDL_GetDisplayBounds(index, &rect);
	return { rect.x, rect.y, rect.w, rect.h };
}

Window::Rect Window::GetDisplayMonitorWorkRect(int index)
{
	SDL_Rect rect;
	SDL_GetDisplayUsableBounds(index, &rect);
	return { rect.x, rect.y, rect.w, rect.h };
}

Window::Window(const void* pParentHandle)
{
	m_pSDLWindow = SDL_CreateWindowFrom(pParentHandle);
}

Window::Window(const char* pTitle, int x, int y, int w, int h)
{
	m_pSDLWindow = SDL_CreateWindow(pTitle, x == -1 ? SDL_WINDOWPOS_UNDEFINED : x, y == -1 ? SDL_WINDOWPOS_UNDEFINED : y, w, h, 0);
}

Window::~Window()
{
	SDL_DestroyWindow(m_pSDLWindow);
}

void* Window::GetHandle() const
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	SDL_GetWindowWMInfo(m_pSDLWindow, &wmi);

#if CD_PLATFORM_OSX || CD_PLATFORM_IOS
	return wmi.info.cocoa.window;
#elif CD_PLATFORM_WINDOWS
	return wmi.info.win.window;
#elif CD_PLATFORM_ANDROID
	return wmi.info.android.window;
#elif CD_PLATFORM_LINUX
	return wmi.info.x11.window;
#else
	static_assert("CD_PLATFORM macro not defined!");
#endif
}

const char* Window::GetTitle() const
{
	return SDL_GetWindowTitle(m_pSDLWindow);
}

void Window::SetTitle(const char* pTitle)
{
	SDL_SetWindowTitle(m_pSDLWindow, pTitle);
}

int Window::GetWidth() const
{
	int width;
	SDL_GetWindowSize(m_pSDLWindow, &width, nullptr);
	return width;
}

int Window::GetHeight() const
{
	int height;
	SDL_GetWindowSize(m_pSDLWindow, nullptr, &height);
	return height;
}

std::pair<int, int> Window::GetSize() const
{
	int width, height;
	SDL_GetWindowSize(m_pSDLWindow, &width, &height);
	return std::make_pair(width, height);
}

void Window::SetSize(int w, int h)
{
	SDL_SetWindowSize(m_pSDLWindow, w, h);

	// Center the window
	SDL_DisplayMode dm;
	SDL_GetDesktopDisplayMode(0, &dm);
	int screenWidth = dm.w;
	int screenHeight = dm.h;
	int windowX = (screenWidth - w) / 2;
	int windowY = (screenHeight - h) / 2;
	SDL_SetWindowPosition(m_pSDLWindow, windowX, windowY);

	OnResize.Invoke(static_cast<uint16_t>(w), static_cast<uint16_t>(h));
}

int Window::GetPositionX() const
{
	int x;
	SDL_GetWindowPosition(m_pSDLWindow, &x, nullptr);
	return x;
}

int Window::GetPositionY() const
{
	int y;
	SDL_GetWindowPosition(m_pSDLWindow, nullptr, &y);
	return y;
}

std::pair<int, int> Window::GetPosition() const
{
	int x, y;
	SDL_GetWindowPosition(m_pSDLWindow, &x, &y);
	return std::make_pair(x, y);
}

void Window::SetPosition(int x, int y)
{
	SDL_SetWindowPosition(m_pSDLWindow, x, y);
}

void Window::Show()
{
	SDL_ShowWindow(m_pSDLWindow);
}

void Window::Hide()
{
	SDL_HideWindow(m_pSDLWindow);
}

void Window::SetFullScreen(bool on)
{
	SDL_SetWindowFullscreen(m_pSDLWindow, on ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

bool Window::IsInputFocused() const
{
	return SDL_GetWindowFlags(m_pSDLWindow) & SDL_WINDOW_INPUT_FOCUS;
}

bool Window::IsMouseFocused() const
{
	return SDL_GetWindowFlags(m_pSDLWindow) & SDL_WINDOW_MOUSE_FOCUS;
}

bool Window::IsFocused() const
{
	return SDL_GetWindowFlags(m_pSDLWindow) & SDL_WINDOW_INPUT_FOCUS;
}

void Window::SetFocused()
{
	SDL_RaiseWindow(m_pSDLWindow);
}

void Window::Close(bool bPushSdlEvent)
{
	m_isClosed = true;
	if (!bPushSdlEvent) { return; }

	SDL_Event sdlEvent;
	SDL_QuitEvent& quitEvent = sdlEvent.quit;
	quitEvent.type = SDL_QUIT;
	SDL_PushEvent(&sdlEvent);
}

bool Window::IsMinimized() const
{
	return SDL_GetWindowFlags(m_pSDLWindow) & SDL_WINDOW_MINIMIZED;
}

void Window::SetResizeable(bool on)
{
	SDL_SetWindowResizable(m_pSDLWindow, static_cast<SDL_bool>(on));
}

void Window::SetBordedLess(bool on)
{
	SDL_SetWindowBordered(m_pSDLWindow, static_cast<SDL_bool>(!on));
}

void Window::SetWindowIcon(const char* pFilePath) const
{
	if (!pFilePath)
	{
		return;
	}

	std::string iconFilePath = CDEDITOR_RESOURCES_ROOT_PATH;
	iconFilePath += pFilePath;

	int width, height, originFormat;
	int depth = 32;
	int channels = STBI_rgb_alpha;
	void* pImageData = stbi_load(iconFilePath.c_str(), &width, &height, &originFormat, STBI_rgb_alpha);
	if (nullptr == pImageData)
	{
		return;
	}

	uint32_t maskR, maskG, maskB, maskA;
	if constexpr (SDL_BYTEORDER == SDL_BIG_ENDIAN)
	{
		maskR = 0xff000000;
		maskG = 0x00ff0000;
		maskB = 0x0000ff00;
		maskA = 0x000000ff;
	}
	else
	{
		maskR = 0x000000ff;
		maskG = 0x0000ff00;
		maskB = 0x00ff0000;
		maskA = 0xff000000;
	}

	SDL_Surface* pSDLSurface = SDL_CreateRGBSurfaceFrom(pImageData, width, height, depth,
		channels * width, maskR, maskG, maskB, maskA);

	SDL_SetWindowIcon(m_pSDLWindow, pSDLSurface);
	SDL_FreeSurface(pSDLSurface);
	stbi_image_free(pImageData);
}

void Window::SetMouseVisible(bool isVisible, uint32_t x, uint32_t y)
{
	SDL_ShowCursor(isVisible);
	if (!isVisible)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
		SDL_WarpMouseInWindow(m_pSDLWindow, x, y);
	}
	else
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
}

void Window::WrapMouseInCenter() const
{
	int w, h;
	SDL_GetWindowSize(m_pSDLWindow, &w, &h);
	SDL_WarpMouseInWindow(m_pSDLWindow, w / 2, h / 2);
}

void Window::Update()
{

}

}