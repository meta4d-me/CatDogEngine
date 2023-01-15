#include "Window.h"

#include "Input.h"

#include <sdl.h>
#include <SDL_syswm.h>
#include <SDL_video.h>
#include <SDL_joystick.h>
#include <SDL_gamecontroller.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <string>

namespace engine
{

Window::Window(const char* pTitle, uint16_t width, uint16_t height)
	: m_width(width)
	, m_height(height)
{
	// JoyStick : SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER
	SDL_Init(SDL_INIT_EVENTS);
	SDL_SetHintWithPriority("SDL_BORDERLESS_RESIZABLE_STYLE", "1", SDL_HINT_OVERRIDE);
	SDL_SetHintWithPriority("SDL_BORDERLESS_WINDOWED_STYLE", "1", SDL_HINT_OVERRIDE);

	// If you want to implement window like Visual Studio without titlebar provided by system OS, open SDL_WINDOW_BORDERLESS.
	// But the issue is that you can't drag it unless you provide an implementation about hit test.
	// Then you also need to simulate minimize and maxmize buttons.
	m_pSDLWindow = SDL_CreateWindow(pTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_RESIZABLE);
	SDL_WarpMouseInWindow(m_pSDLWindow, static_cast<int>(width * 0.5f), static_cast<int>(height * 0.5f));

	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	SDL_GetWindowWMInfo(m_pSDLWindow, &wmi);

#if PLATFORM_OSX || PLATFORM_IOS
	m_pNativeWindowHandle = wmi.info.cocoa.window;
#elif PLATFORM_WINDOWS
	m_pNativeWindowHandle = wmi.info.win.window;
#elif PLATFORM_ANDROID
	m_pNativeWindowHandle = wmi.info.android.window;
#endif
}

Window::~Window()
{
	SDL_Quit();
	SDL_DestroyWindow(m_pSDLWindow);
}

void Window::Closed(bool bPushSdlEvent)
{
	m_IsClosed = true;
	if (!bPushSdlEvent) { return; }

	SDL_Event sdlEvent;
	SDL_QuitEvent& quitEvent = sdlEvent.quit;
	quitEvent.type = SDL_QUIT;
	SDL_PushEvent(&sdlEvent);
}

void Window::Update()
{
	Input::Get().Reset();

	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
		{
			Closed(false);
		}
		break;

		case SDL_WINDOWEVENT:
		{
			const SDL_WindowEvent& wev = sdlEvent.window;
			switch (wev.event)
			{
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					int currentWindowWidth;
					int currentWindowHeight;
					SDL_GetWindowSize(m_pSDLWindow, &currentWindowWidth, &currentWindowHeight);
					if (currentWindowWidth != m_width || currentWindowHeight != m_height)
					{
						m_width = currentWindowWidth;
						m_height = currentWindowHeight;
						SDL_SetWindowSize(m_pSDLWindow, m_width, m_height);

						OnResize.Invoke(m_width, m_height);
					}
				}
				break;
			}
		}
		break;

		case SDL_MOUSEMOTION:
		{
			// Top left is (0,0) for (x, y)
			// xrel is positive to the right, negative to the left
			// yrel is positive to the bottom, negative to the top
			const SDL_MouseMotionEvent& mouseMotionEvent = sdlEvent.motion;
			Input::Get().SetMousePositionX(mouseMotionEvent.x);
			Input::Get().SetMousePositionY(mouseMotionEvent.y);
			Input::Get().SetMousePositionOffsetX(mouseMotionEvent.xrel);
			Input::Get().SetMousePositionOffsetY(mouseMotionEvent.yrel);
		}
		break;

		case SDL_MOUSEBUTTONDOWN:
		{
			switch (sdlEvent.button.button)
			{
			case SDL_BUTTON_LEFT:
				Input::Get().SetMouseLBPressed(true);
				break;
			case SDL_BUTTON_RIGHT:
				Input::Get().SetMouseRBPressed(true);
				break;
			case SDL_BUTTON_MIDDLE:
				Input::Get().SetMouseMBPressed(true);
				break;
			}
		}
		break;

		case SDL_MOUSEBUTTONUP:
		{
			switch (sdlEvent.button.button)
			{
			case SDL_BUTTON_LEFT:
				Input::Get().SetMouseLBPressed(false);
				break;
			case SDL_BUTTON_RIGHT:
				Input::Get().SetMouseRBPressed(false);
				break;
			case SDL_BUTTON_MIDDLE:
				Input::Get().SetMouseMBPressed(false);
				break;
			}
		}
		break;

		case SDL_MOUSEWHEEL:
		{
			Input::Get().SetMouseScrollOffsetY(sdlEvent.wheel.preciseY);
		}
		break;

		case SDL_KEYDOWN:
		{
			Sint32 sdlKeyCode = sdlEvent.key.keysym.sym;

			if (sdlKeyCode < Input::MaxKeyCode)
			{
				Input::Get().SetKeyPressed(static_cast<uint8_t>(sdlKeyCode), true);
			}

			KeyMod keyMod = static_cast<KeyMod>(sdlEvent.key.keysym.mod);
			if (keyMod != KeyMod::KMOD_NONE)
			{
				Input::Get().SetModifier(keyMod);
			}
		}
		break;

		case SDL_KEYUP:
		{
			Sint32 sdlKeyCode = sdlEvent.key.keysym.sym;

			if (sdlKeyCode < Input::MaxKeyCode)
			{
				Input::Get().SetKeyPressed(static_cast<uint8_t>(sdlKeyCode), false);
			}

			KeyMod keyMod = static_cast<KeyMod>(sdlEvent.key.keysym.mod);
			if (keyMod != KeyMod::KMOD_NONE)
			{
				Input::Get().ClearModifier(keyMod);
			}
		}
		break;

		case SDL_DROPFILE:
		{
			OnDropFile.Invoke(sdlEvent.drop.file);
		}
		break;

		default:
			break;
		}
	}
}

void Window::SetWindowIcon(const char* pFilePath) const
{
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
	if constexpr(SDL_BYTEORDER == SDL_BIG_ENDIAN)
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

}