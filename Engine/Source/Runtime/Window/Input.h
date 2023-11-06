#pragma once

#include "KeyCode.h"

#include <array>
#include <cstdint>
#include <vector>

namespace engine
{

// Input system will cache keyboard/mouse/joystick/... status in current frame after processing Window message loop.
// Then other modules can query these status from Input without binding their callbacks directly to Window system.
// It will very useful if you want to implement a complex editor case:
// Editor and Engine are in different ImGui context but you want to dispatch mouse events to them until they are active.
// So you need to save current frame's input status to implement whenever other modules can query input operations after Window message loop.
class Input
{
public:
	// TODO : some keycodes' value is too large to put them into a continues array memory...
	// We will ignore them at first as they are not commonly used in game and editor.
	static constexpr uint8_t MaxKeyCode = 255;
	static constexpr size_t MaxInputCharBuffer = 32;

	struct KeyEvent 
	{
		KeyCode code;
		KeyMod mod;
		bool isPressed;
	};

public:
	// Input is a singleton class which will be used conveniently to query device status.
	// Always call SetXXX methods in the main thread as Window::Update occurs there. So don't worry about thread safe.
	static Input& Get()
	{
		static Input s_instance;
		return s_instance;
	}

	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
	Input(Input&&) = default;
	Input& operator=(Input&&) = default;
	~Input() = default;

	// Some input status such as mouse move offset x/y needs to clear every frame.
	// TODO : consider foreground/background switch.
	void Reset()
	{
		m_mousePositionOffsetX = m_mousePositionOffsetY = 0;
		m_mouseScrollOffsetY = 0;
	}

	// Mouse device
	bool IsMouseLBPressed() const { return m_mouseLBPressed; }
	void SetMouseLBPressed(bool pressed) { m_mouseLBPressed = pressed; }

	bool IsMouseRBPressed() const { return m_mouseRBPressed; }
	void SetMouseRBPressed(bool pressed) { m_mouseRBPressed = pressed; }

	bool IsMouseMBPressed() const { return m_mouseMBPressed; }
	void SetMouseMBPressed(bool pressed) { m_mouseMBPressed = pressed; }

	float GetMouseScrollOffsetY() const { return m_mouseScrollOffsetY; }
	void SetMouseScrollOffsetY(float offset) { m_mouseScrollOffsetY = offset; }

	int32_t GetMousePositionX() const { return m_mousePositionX; }
	void SetMousePositionX(int32_t x) { m_mousePositionX = x; }

	int32_t GetMousePositionY() const { return m_mousePositionY; }
	void SetMousePositionY(int32_t y) { m_mousePositionY = y; }

	int32_t GetMousePositionOffsetX() const { return m_mousePositionOffsetX; }
	void SetMousePositionOffsetX(int32_t x) { m_mousePositionOffsetX = x; }

	int32_t GetMousePositionOffsetY() const { return m_mousePositionOffsetY; }
	void SetMousePositionOffsetY(int32_t y) { m_mousePositionOffsetY = y; }

	// Keyboard device
	bool IsKeyPressed(KeyCode code) const { return m_keyPressed[static_cast<uint8_t>(code)]; }
	void SetKeyPressed(KeyCode code, bool pressed);

	void SetModifier(KeyMod mod);
	void ClearModifier(KeyMod mod);
	bool ContainsModifier(KeyMod mod) const;

	void AppendKeyEvent(KeyCode code, KeyMod mod, bool pressed);
	void AppendInputCharacter(const char* c, size_t len);

	const std::vector<KeyEvent>& GetKeyEventList() const { return m_keyEventList; }
	const char* GetInputCharacters() const { return m_inputCharBuffer.data(); }

	void FlushInputs();

	std::pair<int, int> GetGloalMousePosition() const;

private:
	Input();

private:
	// Mouse device
	int32_t m_mousePositionX = 0;
	int32_t m_mousePositionY = 0;
	int32_t m_mousePositionOffsetX = 0;
	int32_t m_mousePositionOffsetY = 0;
	float m_mouseScrollOffsetY = 0.0f;
	bool m_mouseLBPressed = false;
	bool m_mouseRBPressed = false;
	bool m_mouseMBPressed = false;

	// Keyboard device
	bool m_keyPressed[MaxKeyCode]{};
	KeyMod m_keyModifiers = KeyMod::KMOD_NONE;
	std::vector<KeyEvent> m_keyEventList;
	size_t m_inputCharBufferIndex = 0;
	std::array<char, MaxInputCharBuffer> m_inputCharBuffer;
};

}