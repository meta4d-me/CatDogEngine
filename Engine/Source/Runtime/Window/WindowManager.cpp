#include "WindowManager.h"

#include "Base/NameOf.h"
#include "Base/Template.h"
#include "Input.h"
#include "Window.h"

#include <SDL.h>

#include <cassert>

namespace engine
{

std::pair<int, int> WindowManager::GetGloalMousePosition()
{
	int x, y;
	SDL_GetGlobalMouseState(&x, &y);
	return std::make_pair(x, y);
}

WindowManager::WindowManager()
{
	// JoyStick : SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	SDL_SetHintWithPriority("SDL_BORDERLESS_RESIZABLE_STYLE", "1", SDL_HINT_OVERRIDE);
	SDL_SetHintWithPriority("SDL_BORDERLESS_WINDOWED_STYLE", "1", SDL_HINT_OVERRIDE);

	// Init system cursors.
	m_allMouseCursors.resize(nameof::enum_count<MouseCursorType>(), nullptr);
	m_allMouseCursors[static_cast<int>(MouseCursorType::Arrow)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	m_allMouseCursors[static_cast<int>(MouseCursorType::Crosshair)] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
}

WindowManager::~WindowManager()
{
	m_allWindows.clear();

	for (auto* pCursor : m_allMouseCursors)
	{
		SDL_FreeCursor(pCursor);
	}
	m_allMouseCursors.clear();

	SDL_Quit();
}

Window* WindowManager::GetWindow(uint32_t id) const
{
	auto itWindow = m_allWindows.find(id);
	return itWindow != m_allWindows.end() ? itWindow->second.get() : nullptr;
}

void WindowManager::AddWindow(std::unique_ptr<Window> pWindow)
{
	m_allWindows[pWindow->GetID()] = cd::MoveTemp(pWindow);
}

void WindowManager::RemoveWindow(uint32_t id)
{
	m_allWindows.erase(id);
}

engine::Window* WindowManager::GetActiveWindow() const
{
	for (const auto& [_, pWindow] : m_allWindows)
	{
		if (pWindow->IsFocused())
		{
			return pWindow.get();
		}
	}

	return nullptr;
}

void WindowManager::ShowCursor(bool on) const
{
	SDL_ShowCursor(on);
}

void WindowManager::SetCursor(MouseCursorType cursorType) const
{
	SDL_SetCursor(m_allMouseCursors[static_cast<int>(cursorType)]);
}

void WindowManager::Update()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
		{
			for (const auto& [_, pWindow] : m_allWindows)
			{
				pWindow->Close(false);
			}
		}
		break;

		case SDL_WINDOWEVENT:
		{
			const SDL_WindowEvent& wev = sdlEvent.window;
			switch (wev.event)
			{
			case SDL_WINDOWEVENT_ENTER:
			{
				wev.windowID;
			}
			break;
			case SDL_WINDOWEVENT_LEAVE:
			{
				wev.windowID;
			}
			break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			{
				Input::Get().SetFocused(true);
			}
			break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
			{
				Input::Get().SetFocused(false);
			}
			break;
			case SDL_WINDOWEVENT_CLOSE:
			case SDL_WINDOWEVENT_MOVED:
			case SDL_WINDOWEVENT_RESIZED:
			{
				//if (ImGuiViewport* pViewport = ImGui::FindViewportByPlatformHandle((void*)SDL_GetWindowFromID(wev.windowID)))
				//{
				//	pViewport->PlatformRequestClose = wev.event == SDL_WINDOWEVENT_CLOSE;
				//	pViewport->PlatformRequestMove = wev.event == SDL_WINDOWEVENT_MOVED;
				//	pViewport->PlatformRequestResize = wev.event == SDL_WINDOWEVENT_RESIZED;
				//}
			}
			break;
			}
		}
		break;

		case SDL_MOUSEMOTION:
		{
			const SDL_MouseMotionEvent& mouseMotionEvent = sdlEvent.motion;
			engine::Input::Get().SetMousePositionX(mouseMotionEvent.x);
			engine::Input::Get().SetMousePositionY(mouseMotionEvent.y);
			engine::Input::Get().SetMousePositionOffsetX(mouseMotionEvent.xrel);
			engine::Input::Get().SetMousePositionOffsetY(mouseMotionEvent.yrel);
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

		case SDL_TEXTINPUT:
		{
			const size_t inputLen = strlen(sdlEvent.text.text);
			Input::Get().AppendInputCharacter(sdlEvent.text.text, inputLen);
		}
		break;

		case SDL_KEYDOWN:
		{
			Sint32 sdlKeyCode = sdlEvent.key.keysym.sym;
			KeyMod keyMod = static_cast<KeyMod>(sdlEvent.key.keysym.mod);
			if (keyMod != KeyMod::KMOD_NONE)
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

		case SDL_KEYUP:
		{
			Sint32 sdlKeyCode = sdlEvent.key.keysym.sym;
			KeyMod keyMod = static_cast<KeyMod>(sdlEvent.key.keysym.mod);
			if (keyMod != KeyMod::KMOD_NONE)
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

}