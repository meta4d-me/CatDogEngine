#include "Input.h"

#include <utility>

namespace engine
{

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

}