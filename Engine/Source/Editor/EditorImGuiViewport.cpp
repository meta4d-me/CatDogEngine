#include "EditorImGuiViewport.h"

#include "Display/FlybyCamera.h"
#include "Rendering/PostProcessRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SwapChain.h"
#include "Rendering/SceneRenderer.h"
#include "Rendering/SkyRenderer.h"
#include "Window/Window.h"

#include <imgui/imgui.h>

namespace editor
{

EditorImGuiViewport::EditorImGuiViewport(engine::RenderContext* pRenderContext)
{
	static engine::RenderContext* pRenderContextCache = pRenderContext;

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
	//	engine::Window* pWindow = static_cast<engine::Window*>(pViewport->PlatformHandle);
	//	assert(pWindow && "ImGuiViewport can't find its window handle?");
	//
	//	uint8_t swapChainID = pRenderContextCache->CreateSwapChain(pWindow->GetNativeHandle(), pWindow->GetWidth(), pWindow->GetHeight());
	//	engine::SwapChain* pSwapChain = pRenderContextCache->GetSwapChain(swapChainID);
	//
	//	// TODO : Every renderer should output to a framebuffer, not need to know it is swap chain or gbuffer.
	//	// Refact these logics after editor framework.
	//	int renderTargetWidth = 300;
	//	int renderTargetHeight = 300;
	//	pRenderContextCache->InitGBuffer(renderTargetWidth, renderTargetHeight);
	//	pRenderContextCache->AddRenderer(std::make_unique<engine::SkyRenderer>(pRenderContextCache, pRenderContextCache->CreateView(), pSwapChain));
	//	pRenderContextCache->AddRenderer(std::make_unique<engine::SceneRenderer>(pRenderContextCache, pRenderContextCache->CreateView(), pSwapChain));
	//	pRenderContextCache->AddRenderer(std::make_unique<engine::PostProcessRenderer>(pRenderContextCache, pRenderContextCache->CreateView(), pSwapChain));
	//
	//	// Camera
	//	auto pCamera = std::make_unique<engine::FlybyCamera>(bx::Vec3(0.0f, 0.0f, -50.0f));
	//	pCamera->SetAspect(static_cast<float>(renderTargetWidth) / renderTargetHeight);
	//	pCamera->SetFov(45.0f);
	//	pCamera->SetNearPlane(0.1f);
	//	pCamera->SetFarPlane(1000.0f);
	//	pCamera->SetHomogeneousNdc(bgfx::getCaps()->homogeneousDepth);
	//	pRenderContextCache->SetCamera(pCamera.get());
	//	pWindow->OnResize.Bind<engine::Camera, &engine::Camera::SetAspect>(pCamera.get());
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