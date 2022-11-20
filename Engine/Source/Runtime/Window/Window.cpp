#include "Window.h"

#include <sdl.h>
#include <SDL_syswm.h>
#include <SDL_video.h>
#include <SDL_joystick.h>
#include <SDL_gamecontroller.h>

namespace engine
{

Window::Window(const char* pTitle, uint16_t width, uint16_t height)
	: m_width(width)
	, m_height(height)
{
	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);

	m_pSDLWindow = SDL_CreateWindow(pTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
			const SDL_MouseMotionEvent& mouseMotionEvent = sdlEvent.motion;
			OnMouseMove.Invoke(mouseMotionEvent.x, mouseMotionEvent.y);
			OnMouseMoveRelative.Invoke(mouseMotionEvent.xrel, mouseMotionEvent.yrel);
		}
		break;

		case SDL_MOUSEBUTTONDOWN:
		{
			switch (sdlEvent.button.button)
			{
			case SDL_BUTTON_LEFT:
				OnMouseLBDown.Invoke();
				break;
			case SDL_BUTTON_RIGHT:
				OnMouseRBDown.Invoke();
				break;
			case SDL_BUTTON_MIDDLE:
				OnMouseMBDown.Invoke();
				break;
			}
		}
		break;

		case SDL_MOUSEBUTTONUP:
		{
			switch (sdlEvent.button.button)
			{
			case SDL_BUTTON_LEFT:
				OnMouseLBUp.Invoke();
				break;
			case SDL_BUTTON_RIGHT:
				OnMouseRBUp.Invoke();
				break;
			case SDL_BUTTON_MIDDLE:
				OnMouseMBUp.Invoke();
				break;
			}
		}
		break;

		case SDL_MOUSEWHEEL:
		{
			OnMouseWheel.Invoke(sdlEvent.wheel.preciseY);
		}
		break;

		case SDL_KEYDOWN:
		{
			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
			{
				Closed();
			}

			OnKeyDown.Invoke(sdlEvent.key.keysym.sym, sdlEvent.key.keysym.mod);
		}
		break;

		case SDL_KEYUP:
		{
			OnKeyUp.Invoke(sdlEvent.key.keysym.sym, sdlEvent.key.keysym.mod);
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

bool Window::ShouldClose()
{
	return m_IsClosed;
}

void Window::SetSize(uint16_t width, uint16_t height)
{
	m_width = width;
	m_height = height;
}

}