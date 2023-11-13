#include "Input.h"

#include "Log/Log.h"
#include "SDL.h"

#include <imgui/imgui.h>
#include <unordered_map>
#include <utility>

namespace engine
{

Input::Input()
{
	memset(m_keyPressed, static_cast<int>(false), sizeof(bool) * MaxKeyCode);
}

bool Input::ContainsModifier(KeyMod mod) const
{
	if (m_keyModifiers == KeyMod::KMOD_NONE) {
		return false;
	}
	return static_cast<std::underlying_type_t<KeyMod>>(m_keyModifiers) & static_cast<std::underlying_type_t<KeyMod>>(mod);
}

void Input::SetModifier(KeyMod mod)
{
	if (!ContainsModifier(mod))
	{
		m_keyModifiers = static_cast<KeyMod>(static_cast<std::underlying_type_t<KeyMod>>(m_keyModifiers) | static_cast<std::underlying_type_t<KeyMod>>(mod));
	}
}

void Input::ClearModifier(KeyMod mod)
{
	if (ContainsModifier(mod))
	{
		m_keyModifiers = static_cast<KeyMod>(static_cast<std::underlying_type_t<KeyMod>>(m_keyModifiers) & ~static_cast<std::underlying_type_t<KeyMod>>(mod));
	}
}

void Input::SetKeyPressed(KeyCode code, bool pressed)
{ 
	m_keyPressed[static_cast<std::underlying_type_t<KeyCode>>(code)] = pressed;
}

void Input::AppendKeyEvent(KeyCode code, KeyMod mod, bool pressed)
{
	KeyEvent newEvent;
	newEvent.code = code;
	newEvent.mod = mod;
	newEvent.isPressed = pressed;
	m_keyEventList.push_back(newEvent);
}

void Input::AppendInputCharacter(const char* c, size_t len)
{
	if ((m_inputCharBufferIndex + len) >= MaxInputCharBuffer)
	{
		// overflown for this frame, ignore this input
		return;
	}
	for (size_t i = 0; i < len; ++i)
	{
		m_inputCharBuffer[m_inputCharBufferIndex + i] = c[i];
	}
	m_inputCharBuffer[m_inputCharBufferIndex + len] = '\0';
	m_inputCharBufferIndex += len;
}

void Input::FlushInputs()
{ 
	m_keyEventList.clear(); 
	m_inputCharBufferIndex = 0;
	m_inputCharBuffer[0] = '\0';
}

std::pair<int, int> Input::GetGloalMousePosition() const
{
	int x, y;
	SDL_GetGlobalMouseState(&x, &y);
	return std::make_pair(x, y);
}

void Input::Update()
{
	Input::Get().Reset();
}

}