#pragma once

#include "Core/Delegates/Delegate.hpp"

#include <cstdint>
#include <map>
#include <memory>

namespace engine
{

class Window;

class WindowManager final
{
public:
	using MapIDToWindow = std::map<uint32_t, std::unique_ptr<engine::Window>>;

public:
	WindowManager();
	WindowManager(const WindowManager&) = delete;
	WindowManager& operator=(const WindowManager&) = delete;
	WindowManager(WindowManager&&) = default;
	WindowManager& operator=(WindowManager&&) = default;
	~WindowManager();

	MapIDToWindow& GetAllWindows() { return m_allWindows; }
	const MapIDToWindow& GetAllWindows() const { return m_allWindows; }
	engine::Window* GetWindow(uint32_t id) const;
	void AddWindow(std::unique_ptr<engine::Window> pWindow);
	void RemoveWindow(uint32_t id);

	void Update();

	// Delegates
	Delegate<void(uint32_t, int, int)> OnMouseMove;
	Delegate<void(const char*)> OnDropFile;

private:
	engine::Window* GetActiveWindow() const;

private:
	MapIDToWindow m_allWindows;
};

}