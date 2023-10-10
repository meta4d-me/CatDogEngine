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

Window::Window(const char* pTitle, uint16_t width, uint16_t height, bool useFullScreen)
	: m_width(width)
	, m_height(height)
{
	// If you want to implement window like Visual Studio without titlebar provided by system OS, open SDL_WINDOW_BORDERLESS.
	// But the issue is that you can't drag it unless you provide an implementation about hit test.
	// Then you also need to simulate minimize and maxmize buttons.
	m_pSDLWindow = SDL_CreateWindow(pTitle, width, height, SDL_WINDOW_RESIZABLE);
	SDL_SetWindowFullscreen(m_pSDLWindow, static_cast<SDL_bool>(useFullScreen));
	SDL_WarpMouseInWindow(m_pSDLWindow, width * 0.5f, height * 0.5f);

	SDL_SysWMinfo wmi;
	SDL_GetWindowWMInfo(m_pSDLWindow, &wmi, SDL_SYSWM_CURRENT_VERSION);

#if CD_PLATFORM_OSX || CD_PLATFORM_IOS
	m_pNativeWindowHandle = wmi.info.cocoa.window;
#elif CD_PLATFORM_WINDOWS
	m_pNativeWindowHandle = wmi.info.win.window;
#elif CD_PLATFORM_ANDROID
	m_pNativeWindowHandle = wmi.info.android.window;
#elif CD_PLATFORM_LINUX
	m_pNativeWindowHandle = wmi.info.x11.window;
	// m_pNativeWindowHandle = wmi.info.wl.window;
#else
	static_assert("CD_PLATFORM macro not defined!");
#endif
}

Window::~Window()
{
	SDL_DestroyWindow(m_pSDLWindow);
}

void Window::Close(bool bPushSdlEvent)
{
	m_isClosed = true;
	if (!bPushSdlEvent) { return; }

	SDL_Event sdlEvent;
	SDL_QuitEvent& quitEvent = sdlEvent.quit;
	quitEvent.type = SDL_EVENT_QUIT;
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
		case SDL_EVENT_QUIT:
		{
			Close(false);
		}
		break;

		case SDL_EVENT_WINDOW_RESIZED:
		{
			const SDL_WindowEvent& wev = sdlEvent.window;
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

		case SDL_EVENT_MOUSE_MOTION:
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

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
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

		case SDL_EVENT_MOUSE_BUTTON_UP:
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

		case SDL_EVENT_MOUSE_WHEEL:
		{
			Input::Get().SetMouseScrollOffsetY(sdlEvent.wheel.y);
		}
		break;

		case SDL_EVENT_TEXT_INPUT:
		{   
			const size_t inputLen = strlen(sdlEvent.text.text);
			Input::Get().AppendInputCharacter(sdlEvent.text.text, inputLen);
		}
		break;

		case SDL_EVENT_KEY_DOWN:
		{
			Sint32 sdlKeyCode = sdlEvent.key.keysym.sym;
			KeyMod keyMod = static_cast<KeyMod>(sdlEvent.key.keysym.mod);
			if (sdlEvent.key.keysym.mod != SDL_KMOD_NONE)
			{
				Input::Get().SetModifier(keyMod);
			}

			if (sdlKeyCode >= Input::MaxKeyCode)
			{
				return;
			}
			KeyCode keyCode = static_cast<KeyCode>(static_cast<std::underlying_type_t<KeyCode>>(sdlKeyCode));
			Input::Get().SetKeyPressed(keyCode, true);
			Input::Get().AppendKeyEvent(keyCode, keyMod, true);
		}
		break;

		case SDL_EVENT_KEY_UP:
		{
			Sint32 sdlKeyCode = sdlEvent.key.keysym.sym;
			KeyMod keyMod = static_cast<KeyMod>(sdlEvent.key.keysym.mod);
			if (sdlEvent.key.keysym.mod != SDL_KMOD_NONE)
			{
				Input::Get().ClearModifier(keyMod);
			}

			if (sdlKeyCode >= Input::MaxKeyCode)
			{
				return;
			}
			KeyCode keyCode = static_cast<KeyCode>(static_cast<std::underlying_type_t<KeyCode>>(sdlKeyCode));
			Input::Get().SetKeyPressed(keyCode, false);
			Input::Get().AppendKeyEvent(keyCode, keyMod, false);

		}
		break;

		case SDL_EVENT_DROP_FILE:
		{
			OnDropFile.Invoke(sdlEvent.drop.file);
		}
		break;

		default:
			break;
		}
	}
}

const bool Window::GetInputFocus() const
{
	return SDL_GetWindowFlags(m_pSDLWindow) & SDL_WINDOW_INPUT_FOCUS;
}

const bool Window::GetMouseFocus() const
{
	return SDL_GetWindowFlags(m_pSDLWindow) & SDL_WINDOW_MOUSE_FOCUS;
}

void Window::SetTitle(const char* pTitle)
{
	SDL_SetWindowTitle(m_pSDLWindow, pTitle);
}

void Window::SetFullScreen(bool flag)
{
	SDL_SetWindowFullscreen(m_pSDLWindow, static_cast<SDL_bool>(flag));
}

void Window::SetResizeable(bool flag)
{
	SDL_SetWindowResizable(m_pSDLWindow, static_cast<SDL_bool>(flag));
}

void Window::SetBordedLess(bool flag)
{
	SDL_SetWindowBordered(m_pSDLWindow, static_cast<SDL_bool>(!flag));
}

void Window::SetSize(uint16_t width, uint16_t height)
{
	m_width = width;
	m_height = height;
	SDL_SetWindowSize(m_pSDLWindow, width, height);

	// Center the window
	//const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode();
	//int screenWidth = dm->w;
	//int screenHeight = dm->h;
	//int windowX = (screenWidth - width) / 2;
	//int windowY = (screenHeight - height) / 2;
	//SDL_SetWindowPosition(m_pSDLWindow, windowX, windowY);

	OnResize.Invoke(width, height);
}

void Window::SetWindowIcon(const char* pFilePath) const
{
	//if (!pFilePath)
	//{
	//	return;
	//}
	//
	//std::string iconFilePath = CDEDITOR_RESOURCES_ROOT_PATH;
	//iconFilePath += pFilePath;
	//
	//int width, height, originFormat;
	//int depth = 32;
	//int channels = STBI_rgb_alpha;
	//void* pImageData = stbi_load(iconFilePath.c_str(), &width, &height, &originFormat, STBI_rgb_alpha);
	//if (nullptr == pImageData)
	//{
	//	return;
	//}
	//
	//uint32_t maskR, maskG, maskB, maskA;
	//if constexpr(SDL_BYTEORDER == SDL_BIG_ENDIAN)
	//{
	//	maskR = 0xff000000;
	//	maskG = 0x00ff0000;
	//	maskB = 0x0000ff00;
	//	maskA = 0x000000ff;
	//}
	//else
	//{
	//	maskR = 0x000000ff;
	//	maskG = 0x0000ff00;
	//	maskB = 0x00ff0000;
	//	maskA = 0xff000000;
	//}
	//
	//SDL_Surface* pSDLSurface = SDL_CreateRGBSurfaceFrom(pImageData, width, height, depth,
	//	channels * width, maskR, maskG, maskB, maskA);
	//
	//SDL_SetWindowIcon(m_pSDLWindow, pSDLSurface);
	//SDL_FreeSurface(pSDLSurface);
	//stbi_image_free(pImageData);
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

}