#include "WindowManager.h"

#include "Base/Template.h"
#include "Window.h"

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

void WindowManager::Update()
{
	for (auto& [_, pWindow] : m_mapWindows)
	{
		pWindow->Update();
	}
}

}