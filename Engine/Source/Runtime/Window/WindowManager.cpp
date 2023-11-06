#include "WindowManager.h"

#include "Base/Template.h"
#include "Input.h"
#include "Window.h"

// TODO : remove imgui dependencies.
#include "ImGui/imgui.h"

#include "SDL.h"

#include <cassert>

namespace engine
{

WindowManager::WindowManager()
{
	Window::Init();
}

WindowManager::~WindowManager()
{
	Window::Shutdown();
}

Window* WindowManager::GetWindow(void* handle) const
{
	auto itWindow = m_mapWindows.find(handle);
	return itWindow != m_mapWindows.end() ? itWindow->second.get() : nullptr;
}

void WindowManager::AddWindow(std::unique_ptr<Window> pWindow)
{
	m_mapWindows[pWindow->GetHandle()] = cd::MoveTemp(pWindow);
}

void WindowManager::RemoveWindow(void* handle)
{
	m_mapWindows.erase(handle);
}

engine::Window* WindowManager::GetActiveWindow() const
{
	for (const auto& [_, pWindow] : m_mapWindows)
	{
		if (pWindow->IsFocused())
		{
			return pWindow.get();
		}
	}

	return nullptr;
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
			for (const auto& [_, pWindow] : m_mapWindows)
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
				if (ImGuiViewport* pViewport = ImGui::FindViewportByPlatformHandle((void*)SDL_GetWindowFromID(wev.windowID)))
				{
					pViewport->PlatformRequestClose = wev.event == SDL_WINDOWEVENT_CLOSE;
					pViewport->PlatformRequestMove = wev.event == SDL_WINDOWEVENT_MOVED;
					pViewport->PlatformRequestResize = wev.event == SDL_WINDOWEVENT_RESIZED;
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
			int mouseX = mouseMotionEvent.x;
			int mouseY = mouseMotionEvent.y;
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				int windowX, windowY;
				SDL_GetWindowPosition(SDL_GetWindowFromID(mouseMotionEvent.windowID), &windowX, &windowY);
				mouseX += windowX;
				mouseY += windowY;
			}
			Input::Get().SetMousePositionX(mouseX);
			Input::Get().SetMousePositionY(mouseY);
			//Input::Get().SetMousePositionOffsetX(mouseMotionEvent.xrel);
			//Input::Get().SetMousePositionOffsetY(mouseMotionEvent.yrel);
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
			GetActiveWindow()->OnDropFile.Invoke(sdlEvent.drop.file);
		}
		break;

		default:
			break;
		}
	}
}

}