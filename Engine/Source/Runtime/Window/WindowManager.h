#pragma once

#include <map>
#include <memory>

namespace engine
{

class Window;

class WindowManager final
{
public:
	WindowManager();
	WindowManager(const WindowManager&) = delete;
	WindowManager& operator=(const WindowManager&) = delete;
	WindowManager(WindowManager&&) = default;
	WindowManager& operator=(WindowManager&&) = default;
	~WindowManager();

	engine::Window* GetWindow(void* handle) const;
	void AddWindow(std::unique_ptr<engine::Window> pWindow);
	void RemoveWindow(void* handle);

	void Update();

private:
	std::map<void*, std::unique_ptr<engine::Window>> m_mapWindows;
};

}