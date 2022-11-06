#include "PlatformWindow.h"

#include <sdl.h>
#include <SDL_syswm.h>
#include <SDL_video.h>
#include <SDL_joystick.h>
#include <SDL_gamecontroller.h>

namespace engine
{

PlatformWindow::PlatformWindow(const char* pTitle, uint16_t width, uint16_t height)
	: m_width(width)
	, m_height(height)
{
	SDL_Init(SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);

	m_pSDLWindow = SDL_CreateWindow(pTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
	m_pNativeWindowHandle = wmi.info.win.window;
	m_IsClosed = false;
}

PlatformWindow::~PlatformWindow()
{
	SDL_Quit();
	SDL_DestroyWindow(m_pSDLWindow);
}

void PlatformWindow::Closed(bool bPushSdlEvent)
{
	m_IsClosed = true;
	if (!bPushSdlEvent) { return; }

	SDL_Event sdlEvent;
	SDL_QuitEvent& quitEvent = sdlEvent.quit;
	quitEvent.type = SDL_QUIT;
	SDL_PushEvent(&sdlEvent);
}

void PlatformWindow::Update()
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
			OnMouseMotion.Invoke(mouseMotionEvent.xrel, mouseMotionEvent.yrel);
		}
		break;

		case SDL_MOUSEBUTTONDOWN:
		{
			OnMouseDown.Invoke(sdlEvent.button.button, sdlEvent.button.clicks);
		}
		break;

		case SDL_MOUSEBUTTONUP:
		{
			OnMouseUp.Invoke(sdlEvent.button.button, sdlEvent.button.clicks);
		}
		break;

		case SDL_MOUSEWHEEL:
		{
			//OnMouseWheel.Invoke(sdlEvent.wheel);
		}
		break;

		case SDL_TEXTINPUT:
		{
			//OnTextInput.Invoke(sdlEvent.text);
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
			//OnDropFile.Invoke(sdlEvent.drop);
		}
		break;

		case SDL_JOYAXISMOTION:
		{
			//OnJoyStickAxis.Invoke(sdlEvent.jaxis);
		}
		break;

		case SDL_JOYBALLMOTION:
		{
			//OnJoyStickBall.Invoke(sdlEvent.jball);
		}
		break;

		case SDL_JOYHATMOTION:
		{
			//OnJoyStickHat.Invoke(sdlEvent.jhat);
		}
		break;

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		{
			//OnJoyStickButton.Invoke(sdlEvent.jbutton);
		}
		break;

		case SDL_JOYDEVICEADDED:
		{
			//OnJoyStickDevice.Invoke(sdlEvent.jdevice);
		}
		break;

		case SDL_JOYDEVICEREMOVED:
		{
			//OnJoyStickDevice.Invoke(sdlEvent.jdevice);
		}
		break;

		case SDL_CONTROLLERAXISMOTION:
		{
			//OnControllerAxis.Invoke(sdlEvent.caxis);
		}
		break;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
		{
			//OnControllerButton.Invoke(sdlEvent.cbutton);
		}
		break;

		case SDL_CONTROLLERDEVICEADDED:
		{
			/*for (int i = 0, num = SDL_NumJoysticks(); i < num; ++i)
			{
				if (SDL_IsGameController(i))
				{
					// Use the first avaiable game controller.
					// m_pJoySticker = SDL_JoystickOpen(i);
					m_pController = SDL_GameControllerOpen(i);
					logger::info(SDL_GameControllerMapping(m_pController));
					break;
				}
			}*/
		}
		break;

		case SDL_CONTROLLERDEVICEREMOVED:
		{
			/*if (m_pJoySticker)
			{
				SDL_JoystickClose(m_pJoySticker);
				m_pJoySticker = nullptr;
			}

			if (m_pController)
			{
				SDL_GameControllerClose(m_pController);
				m_pController = nullptr;
			}*/
		}
		break;

		case SDL_CONTROLLERDEVICEREMAPPED:
		{

		}
		break;

		case SDL_CONTROLLERTOUCHPADDOWN:
		case SDL_CONTROLLERTOUCHPADMOTION:
		case SDL_CONTROLLERTOUCHPADUP:
		{
			//OnControllerTouchpad.Invoke(sdlEvent.ctouchpad);
		}
		break;

		case SDL_CONTROLLERSENSORUPDATE:
		{
			//OnControllerSensor.Invoke(sdlEvent.csensor);
		}
		break;

		default:
			break;
		}
	}
}

bool PlatformWindow::ShouldClose()
{
	return m_IsClosed;
}

void PlatformWindow::SetSize(uint16_t width, uint16_t height)
{
	m_width = width;
	m_height = height;
}

}