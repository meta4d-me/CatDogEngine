#include "EditorImGuiViewport.h"

#include "EditorApp.h"
#include "Rendering/RenderContext.h"
#include "Window/Window.h"

#include <imgui/imgui.h>

namespace editor
{

EditorImGuiViewport::EditorImGuiViewport(editor::EditorApp* pEditor, engine::RenderContext* pRenderContext)
{
	static editor::EditorApp* pEditorApp = pEditor;

	ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
	platformIO.Platform_CreateWindow = [](ImGuiViewport* pViewport)
	{
		auto pWindow = std::make_unique<engine::Window>("Dockable", 100, 100);
		pViewport->PlatformHandle = pWindow.get();
		pViewport->PlatformHandleRaw = pWindow->GetHandle();
		pEditorApp->AddWindow(cd::MoveTemp(pWindow));
	};
	
	platformIO.Platform_DestroyWindow = [](ImGuiViewport* pViewport)
	{
		pEditorApp->RemoveWindow(pViewport->PlatformHandleRaw);
		pViewport->PlatformHandle = nullptr;
		pViewport->PlatformUserData = nullptr;
	};
	
	platformIO.Platform_ShowWindow = [](ImGuiViewport* pViewport)
	{
		pEditorApp->GetWindow(pViewport->PlatformHandleRaw)->Show();
	};
	
	platformIO.Platform_SetWindowPos = [](ImGuiViewport* pViewport, ImVec2 v)
	{
		pEditorApp->GetWindow(pViewport->PlatformHandleRaw)->SetPosition(static_cast<int>(v.x), static_cast<int>(v.y));
	};
	
	platformIO.Platform_GetWindowPos = [](ImGuiViewport* pViewport) -> ImVec2
	{
		engine::Window* pWindow = pEditorApp->GetWindow(pViewport->PlatformHandleRaw);
		auto position = pWindow->GetPosition();
		return { static_cast<float>(position.first), static_cast<float>(position.second) };
	};
	
	platformIO.Platform_SetWindowSize = [](ImGuiViewport* pViewport, ImVec2 v)
	{
		pEditorApp->GetWindow(pViewport->PlatformHandleRaw)->SetSize(static_cast<int>(v.x), static_cast<int>(v.y));
	};
	
	platformIO.Platform_GetWindowSize = [](ImGuiViewport* pViewport) -> ImVec2
	{
		engine::Window* pWindow = pEditorApp->GetWindow(pViewport->PlatformHandleRaw);
		auto size = pWindow->GetSize();
		return { static_cast<float>(size.first), static_cast<float>(size.second) };
	};
	
	platformIO.Platform_SetWindowTitle = [](ImGuiViewport* pViewport, const char* pTitle)
	{
		engine::Window* pWindow = pEditorApp->GetWindow(pViewport->PlatformHandleRaw);
		pWindow->SetTitle(pTitle);
	};
}

void EditorImGuiViewport::Update()
{
	ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
	int monitorCount = engine::Window::GetDisplayMonitorCount();
	platformIO.Monitors.resize(0);
	for (int monitorIndex = 0; monitorIndex < monitorCount; ++monitorIndex)
	{
		auto mainRect = engine::Window::GetDisplayMonitorMainRect(monitorIndex);
		auto workRect = engine::Window::GetDisplayMonitorWorkRect(monitorIndex);

		ImGuiPlatformMonitor monitor;
		monitor.MainPos = ImVec2((float)mainRect.x, (float)mainRect.y);
		monitor.MainSize = ImVec2((float)mainRect.w, (float)mainRect.h);
		monitor.WorkPos = ImVec2((float)workRect.x, (float)workRect.y);
		monitor.WorkSize = ImVec2((float)workRect.w, (float)workRect.h);

		// Check if the display's position is at (0,0), which is typical for the primary display.
		bool isPrimaryDisplay = mainRect.x == 0 && mainRect.y == 0;
		if (isPrimaryDisplay)
		{
			platformIO.Monitors.push_front(monitor);
		}
		else
		{
			platformIO.Monitors.push_back(monitor);
		}
	}
}

}