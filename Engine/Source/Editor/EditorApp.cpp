#include "EditorApp.h"

#include "Application/Engine.h"
#include "AssetBrowser.h"
#include "DesignView.h"
#include "Display/FlybyCamera.h"
#include "EditorImGuiContext.h"
#include "EditorImGuiViewport.h"
#include "EditorRenderer.h"
#include "MainMenu.h"
#include "Rendering/GBuffer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SwapChain.h"
#include "Rendering/SkyRenderer.h"
#include "Rendering/SceneRenderer.h"
#include "Window/Window.h"

#include <imgui/imgui.h>

#include <format>

namespace editor
{

EditorApp::EditorApp()
{
}

EditorApp::~EditorApp()
{
}

void EditorApp::Init(engine::EngineInitArgs initArgs)
{
	uint16_t width = initArgs.width;
	uint16_t height = initArgs.height;
	AddWindow(std::make_unique<engine::Window>(initArgs.pTitle, width, height));

	GetMainWindow()->SetWindowIcon(initArgs.pIconFilePath);

	InitImGuiContext(initArgs.language);

	// EditorImGuiRenderer is based on ImGui, so it should be behind EditorImGuiContext.
	InitRenderContext();

	// Viewports depend on window and rendering setup.
	InitImGuiViewports(m_pRenderContext);
}

void EditorApp::Shutdown()
{
}

engine::Window* EditorApp::GetWindow(size_t index) const
{
	return m_pAllWindows[index].get();
}

engine::Window* EditorApp::GetMainWindow() const
{
	return GetWindow(0);
}

size_t EditorApp::AddWindow(std::unique_ptr<engine::Window> pWindow)
{
	size_t windowIndex = m_pAllWindows.size();
	m_pAllWindows.emplace_back(std::move(pWindow));
	return windowIndex;
}

void EditorApp::InitImGuiContext(engine::Language language)
{
	assert(GetMainWindow() && "Init window before imgui context");

	m_pEditorImGuiContext = std::make_unique<EditorImGuiContext>();

	// TODO : more font files to load and switch dynamicly.
	std::vector<std::string> ttfFileNames = { "FanWunMing-SB.ttf" };
	m_pEditorImGuiContext->LoadFontFiles(ttfFileNames, language);

	// Init child UI components.
	m_pEditorImGuiContext->AddLayer(std::make_unique<MainMenu>());

	//size_t assetBrowserIndex = AddWindow(std::make_unique<engine::Window>("AssetBrowser", 400, 200));
	m_pEditorImGuiContext->AddLayer(std::make_unique<AssetBrowser>());

	//size_t designViewIndex = AddWindow(std::make_unique<engine::Window>("DesignView", 300, 300));
	m_pEditorImGuiContext->AddLayer(std::make_unique<DesignView>());

	GetMainWindow()->OnMouseLBDown.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseLBDown>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseLBUp.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseLBUp>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseRBDown.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseRBDown>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseRBUp.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseRBUp>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseMBDown.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseMBDown>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseMBUp.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseMBUp>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseWheel.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseWheel>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseMove.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseMove>(m_pEditorImGuiContext.get());
	//GetMainWindow()->OnKeyDown.Bind<EditorImGuiContext, &EditorImGuiContext::OnKeyPress>(m_pEditorImGuiContext.get());
	//GetMainWindow()->OnKeyUp.Bind<EditorImGuiContext, &EditorImGuiContext::OnKeyRelease>(m_pEditorImGuiContext.get());
}

void EditorApp::InitImGuiViewports(engine::RenderContext* pRenderContext)
{
	if (ImGuiViewport* pMainViewport = ImGui::GetMainViewport())
	{
		pMainViewport->PlatformHandle = GetMainWindow();
		pMainViewport->PlatformHandleRaw = GetMainWindow()->GetNativeHandle();
	}
	m_pEditorImGuiViewport = std::make_unique<EditorImGuiViewport>(pRenderContext);
}

void EditorApp::InitRenderContext()
{
	m_pRenderContext = m_pEngine->GetRenderContext();
	GetMainWindow()->OnResize.Bind<engine::RenderContext, &engine::RenderContext::ResizeFrameBuffers>(m_pRenderContext);

	uint8_t mainViewSwapChainID = m_pRenderContext->CreateSwapChain(GetMainWindow()->GetNativeHandle(), GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight());
	engine::SwapChain* pMainViewSwapChain = m_pRenderContext->GetSwapChain(mainViewSwapChainID);
	m_pRenderContext->AddRenderer(std::make_unique<editor::EditorRenderer>(m_pRenderContext, m_pRenderContext->CreateView(), pMainViewSwapChain));
}

bool EditorApp::Update(float deltaTime)
{
	GetMainWindow()->Update();

	m_pEditorImGuiContext->Update();

	m_pEditorImGuiViewport->Update();

	m_pRenderContext->Update(deltaTime);
	
	return !GetMainWindow()->ShouldClose();
}

}