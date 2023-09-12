#include "EditorImGuiViewport.h"

#include "Rendering/RenderContext.h"
#include "Window/Window.h"

#include <imgui/imgui.h>

namespace editor
{

EditorImGuiViewport::EditorImGuiViewport(engine::RenderContext* pRenderContext)
{
	//ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
	//platformIO.Platform_CreateWindow = [](ImGuiViewport* pViewport)
	//{
	//
	//};
	//
	//platformIO.Platform_DestroyWindow = [](ImGuiViewport* pViewport)
	//{
	//
	//};
	//
	//platformIO.Platform_ShowWindow = [](ImGuiViewport* pViewport)
	//{
	//
	//};
	//
	//platformIO.Platform_SetWindowPos = [](ImGuiViewport* pViewport, ImVec2 pos)
	//{
	//
	//};
	//
	//platformIO.Platform_GetWindowPos = [](ImGuiViewport* pViewport) -> ImVec2
	//{
	//	//engine::Window* pWindow = static_cast<engine::Window*>(pViewport->PlatformHandle);
	//	return { 0.0f, 0.0f };
	//};
	//
	//platformIO.Platform_SetWindowSize = [](ImGuiViewport* pViewport, ImVec2 pos)
	//{
	//
	//};
	//
	//platformIO.Platform_GetWindowSize = [](ImGuiViewport* pViewport) -> ImVec2
	//{
	//
	//};
	//
	//platformIO.Platform_SetWindowTitle = [](ImGuiViewport* pViewport, const char* title)
	//{
	//	engine::Window* pWindow = static_cast<engine::Window*>(pViewport->PlatformHandle);
	//};
	//
	//platformIO.Platform_GetWindowFocus = [](ImGuiViewport* pViewport)
	//{
	//
	//};
	//
	//platformIO.Platform_SetWindowFocus = nullptr;

	//platformIO.Renderer_CreateWindow = [](ImGuiViewport* pViewport)
	//{
	//};

	//platformIO.Renderer_DestroyWindow = [](ImGuiViewport* pViewport)
	//{
	//
	//};
	//
	//platformIO.Renderer_SetWindowSize = [](ImGuiViewport* viewport)
	//{
	//
	//};
	//
	//platformIO.Renderer_RenderWindow = [](ImGuiViewport* viewport)
	//{
	//
	//};
	//
	//platformIO.Renderer_SwapBuffers = [](ImGuiViewport* viewport)
	//{
	//
	//};
}

void EditorImGuiViewport::Update()
{
	//struct Rect { int left, top, width, height; };
	//struct Monitor {
	//	Rect work_rect;
	//	Rect monitor_rect;
	//	bool primary;
	//};
	//
	//Monitor monitors[32];
	//const uint32_t monitor_count = os::getMonitors(Span(monitors));
	//ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
	//pio.Monitors.resize(0);
	//for (u32 i = 0; i < monitor_count; ++i) {
	//	const os::Monitor& m = monitors[i];
	//	ImGuiPlatformMonitor im;
	//	im.MainPos = ImVec2((float)m.monitor_rect.left, (float)m.monitor_rect.top);
	//	im.MainSize = ImVec2((float)m.monitor_rect.width, (float)m.monitor_rect.height);
	//	im.WorkPos = ImVec2((float)m.work_rect.left, (float)m.work_rect.top);
	//	im.WorkSize = ImVec2((float)m.work_rect.width, (float)m.work_rect.height);
	//
	//	if (m.primary) {
	//		pio.Monitors.push_front(im);
	//	}
	//	else {
	//		pio.Monitors.push_back(im);
	//	}
	//}
}

}