#include "EditorApp.h"

#include "Application/Engine.h"
#include "Display/FlybyCamera.h"
#include "EditorImGuiContext.h"
#include "EditorImGuiViewport.h"
#include "Rendering/ImGuiRenderer.h"
#include "Rendering/GBuffer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SwapChain.h"
#include "UILayers/AssetBrowser.h"
#include "UILayers/EntityList.h"
#include "UILayers/GameView.h"
#include "UILayers/Inspector.h"
#include "UILayers/MainMenu.h"
#include "UILayers/OutputLog.h"
#include "UILayers/SceneView.h"
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

	// Enable multiple viewports which means that we can drop any imgui window outside the main window.
	// Then the imgui window will become a new standalone window. Drop back to convert it back.
	// InitImGuiViewports(m_pRenderContext);

	// Init static UI layers which mean they are not dockable and dynamic to move.
	m_pEditorImGuiContext->AddStaticLayer(std::make_unique<MainMenu>("MainMenu", this));

	// Init dockable layers.
	m_pEditorImGuiContext->AddDockableLayer(std::make_unique<EntityList>("EntityList", this));

	//m_pEditorImGuiContext->AddDockableLayer(std::make_unique<GameView>("GameView", this));
	m_pEditorImGuiContext->AddDockableLayer(std::make_unique<SceneView>("SceneView", this));

	m_pEditorImGuiContext->AddDockableLayer(std::make_unique<Inspector>("Inspector", this));

	m_pEditorImGuiContext->AddDockableLayer(std::make_unique<AssetBrowser>("AssetBrowser", this));
	m_pEditorImGuiContext->AddDockableLayer(std::make_unique<OutputLog>("OutputLog", this));
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
	m_pRenderContext->AddRenderer(std::make_unique<engine::ImGuiRenderer>(m_pRenderContext, m_pRenderContext->CreateView(), pMainViewSwapChain));

	// Camera is prepared for other renderers except ImGuiRenderer.
	m_pCamera = std::make_unique<engine::FlybyCamera>(bx::Vec3(0.0f, 0.0f, -50.0f));
	m_pCamera->SetAspect(1.0f);
	m_pCamera->SetFov(45.0f);
	m_pCamera->SetNearPlane(0.1f);
	m_pCamera->SetFarPlane(1000.0f);
	m_pCamera->SetHomogeneousNdc(bgfx::getCaps()->homogeneousDepth);
	m_pRenderContext->SetCamera(m_pCamera.get());
	GetMainWindow()->OnResize.Bind<engine::Camera, &engine::Camera::SetAspect>(m_pCamera.get());
}

bool EditorApp::Update(float deltaTime)
{
	GetMainWindow()->Update();

	m_pEditorImGuiContext->Update();

	//m_pEditorImGuiViewport->Update();

	if (m_pCamera)
	{
		m_pCamera->Update();
	}

	m_pRenderContext->Update(deltaTime);
	
	return !GetMainWindow()->ShouldClose();
}

}