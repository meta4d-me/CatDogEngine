#pragma once

#include "Core/Delegates/Delegate.hpp"

#include <cstdint>
#include <map>
#include <memory>

struct SDL_Cursor;

namespace engine
{

class Window;

enum class MouseCursorType : int
{
	Arrow = 0,
	Crosshair
};

class WindowManager final
{
public:
	using MapIDToWindow = std::map<uint32_t, std::unique_ptr<engine::Window>>;

public:
	static std::pair<int, int> GetGloalMousePosition();

public:
	WindowManager();
	WindowManager(const WindowManager&) = delete;
	WindowManager& operator=(const WindowManager&) = delete;
	WindowManager(WindowManager&&) = default;
	WindowManager& operator=(WindowManager&&) = default;
	~WindowManager();

	MapIDToWindow& GetAllWindows() { return m_allWindows; }
	const MapIDToWindow& GetAllWindows() const { return m_allWindows; }
	Window* GetWindow(uint32_t id) const;
	void AddWindow(std::unique_ptr<Window> pWindow);
	void RemoveWindow(uint32_t id);

	const SDL_Cursor* GetMouseCursor(MouseCursorType cursorType) const { return m_allMouseCursors[static_cast<int>(cursorType)]; }
	void ShowCursor(bool on) const;
	void SetCursor(MouseCursorType cursorType) const;

	void Update();

	// Delegates
	Delegate<void(const char*)> OnDropFile;

private:
	engine::Window* GetActiveWindow() const;

private:
	MapIDToWindow m_allWindows;
	std::vector<SDL_Cursor*> m_allMouseCursors;
};

}