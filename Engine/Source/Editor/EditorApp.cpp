﻿#include "EditorApp.h"

#include "Application/Engine.h"
#include "Display/FlybyCamera.h"
#include "EditorImGuiViewport.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/UILayers/DebugPanel.h"
#include "Rendering/ImGuiRenderer.h"
#include "Rendering/PBRSkyRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SceneRenderer.h"
#include "UILayers/AssetBrowser.h"
#include "UILayers/EntityList.h"
#include "UILayers/GameView.h"
#include "UILayers/Inspector.h"
#include "UILayers/MainMenu.h"
#include "UILayers/OutputLog.h"
#include "UILayers/SceneView.h"
#include "Window/Window.h"

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

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

	InitRenderContext();

	// Init ImGuiContext in the editor side which used to draw editor ui.
	InitEditorImGuiContext(initArgs.language);

	// Init ImGuiContext in the engine side which used to draw in game ui.
	InitEngineImGuiContext(initArgs.language);

	// Enable multiple viewports which means that we can drop any imgui window outside the main window.
	// Then the imgui window will become a new standalone window. Drop back to convert it back.
	// TODO : should be only used in the editor ImGuiContext.
	// InitImGuiViewports(m_pRenderContext);
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

void EditorApp::InitEditorImGuiContext(engine::Language language)
{
	assert(GetMainWindow() && "Init window before imgui context");

	m_pEditorImGuiContext = std::make_unique<engine::ImGuiContextInstance>(GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight());

	// TODO : more font files to load and switch dynamicly.
	std::vector<std::string> ttfFileNames = { "FanWunMing-SB.ttf" };
	m_pEditorImGuiContext->LoadFontFiles(ttfFileNames, language);

	// Bind event callbacks from current available input devices.
	GetMainWindow()->OnMouseLBDown.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnMouseLBDown>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseLBUp.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnMouseLBUp>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseRBDown.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnMouseRBDown>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseRBUp.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnMouseRBUp>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseMBDown.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnMouseMBDown>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseMBUp.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnMouseMBUp>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseWheel.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnMouseWheel>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnMouseMove.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnMouseMove>(m_pEditorImGuiContext.get());
	GetMainWindow()->OnResize.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnResize>(m_pEditorImGuiContext.get());
	//GetMainWindow()->OnKeyDown.Bind<ImGuiContextInstance, &ImGuiContextInstance::OnKeyPress>(m_pEditorImGuiContext.get());
	//GetMainWindow()->OnKeyUp.Bind<ImGuiContextInstance, &ImGuiContextInstance::OnKeyRelease>(m_pEditorImGuiContext.get());

	// Set style settings.
	m_pEditorImGuiContext->SetImGuiThemeColor(engine::ThemeColor::Dark);

	ImGui::GetIO().BackendRendererUserData = m_pRenderContext.get();

	// Add UI layers after finish imgui and rendering contexts' initialization.
	m_pEditorImGuiContext->AddStaticLayer(std::make_unique<MainMenu>("MainMenu"));
	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<EntityList>("EntityList"));
	//m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<GameView>("GameView"));
	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<SceneView>("SceneView"));
	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<Inspector>("Inspector"));
	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<AssetBrowser>("AssetBrowser"));
	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<OutputLog>("OutputLog"));
}

void EditorApp::InitEngineImGuiContext(engine::Language language)
{
	constexpr engine::StringCrc sceneRenderTarget("SceneRenderTarget");
	engine::RenderTarget* pSceneRenderTarget = m_pRenderContext->GetRenderTarget(sceneRenderTarget);

	m_pEngineImGuiContext = std::make_unique<engine::ImGuiContextInstance>(pSceneRenderTarget->GetWidth(), pSceneRenderTarget->GetHeight());

	std::vector<std::string> ttfFileNames = { "FanWunMing-SB.ttf" };
	m_pEngineImGuiContext->LoadFontFiles(ttfFileNames, language);

	// Set style settings.
	m_pEngineImGuiContext->SetImGuiThemeColor(engine::ThemeColor::Light);

	ImGui::GetIO().BackendRendererUserData = m_pRenderContext.get();

	m_pEngineImGuiContext->AddDynamicLayer(std::make_unique<engine::DebugPanel>("DebugPanel"));

	pSceneRenderTarget->OnResize.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnResize>(m_pEngineImGuiContext.get());
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
	m_pRenderContext = std::make_unique<engine::RenderContext>();
	m_pRenderContext->Init();

	GetMainWindow()->OnResize.Bind<engine::RenderContext, &engine::RenderContext::OnResize>(m_pRenderContext.get());

	constexpr engine::StringCrc editorSwapChainName("EditorUISwapChain");
	engine::RenderTarget* pRenderTarget = m_pRenderContext->CreateRenderTarget(editorSwapChainName, GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight(), GetMainWindow()->GetNativeHandle());
	AddEditorRenderer(std::make_unique<engine::ImGuiRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pRenderTarget));

	// Camera is prepared for other renderers except ImGuiRenderer.
	m_pCamera = std::make_unique<engine::FlybyCamera>(bx::Vec3(0.0f, 0.0f, -50.0f));
	m_pCamera->SetAspect(1.0f);
	m_pCamera->SetFov(45.0f);
	m_pCamera->SetNearPlane(0.1f);
	m_pCamera->SetFarPlane(1000.0f);
	m_pCamera->SetHomogeneousNdc(bgfx::getCaps()->homogeneousDepth);
	m_pRenderContext->SetCamera(m_pCamera.get());

	// The init size doesn't make sense. It will resize by SceneView;
	uint16_t width = 800;
	uint16_t height = 800;
	constexpr engine::StringCrc sceneViewRenderTargetName("SceneRenderTarget");
	std::vector<engine::AttachmentDescriptor> attachmentDesc = {
		{ .textureFormat = engine::TextureFormat::RGBA32F },
		{ .textureFormat = engine::TextureFormat::RGBA32F },
		{ .textureFormat = engine::TextureFormat::D32F },
	};
	engine::RenderTarget* pSceneRenderTarget = m_pRenderContext->CreateRenderTarget(sceneViewRenderTargetName, width, height, std::move(attachmentDesc));
	pSceneRenderTarget->OnResize.Bind<engine::Camera, &engine::Camera::SetAspect>(m_pCamera.get());
	AddEngineRenderer(std::make_unique<engine::PBRSkyRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pSceneRenderTarget));
	AddEngineRenderer(std::make_unique<engine::SceneRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pSceneRenderTarget));

	// Note that if you don't want to use ImGuiRenderer for engine, you should also disable EngineImGuiContext.
	AddEngineRenderer(std::make_unique<engine::ImGuiRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pSceneRenderTarget));
}

void EditorApp::AddEditorRenderer(std::unique_ptr<engine::Renderer> pRenderer)
{
	pRenderer->Init();
	m_pEditorRenderers.emplace_back(std::move(pRenderer));
}

void EditorApp::AddEngineRenderer(std::unique_ptr<engine::Renderer> pRenderer)
{
	pRenderer->Init();
	pRenderer->SetCamera(m_pCamera.get());
	m_pEngineRenderers.emplace_back(std::move(pRenderer));
}

bool EditorApp::Update(float deltaTime)
{
	m_pEditorImGuiContext->SwitchCurrentContext();

	GetMainWindow()->Update();

	m_pEditorImGuiContext->Update();

	m_pRenderContext->BeginFrame();
	for (std::unique_ptr<engine::Renderer>& pRenderer : m_pEditorRenderers)
	{
		const float* pViewMatrix = nullptr;
		const float* pProjectionMatrix = nullptr;
		if (engine::Camera* pCamera = pRenderer->GetCamera())
		{
			pCamera->Update();
			pViewMatrix = pCamera->GetViewMatrix();
			pProjectionMatrix = pCamera->GetProjectionMatrix();
		}

		pRenderer->UpdateView(pViewMatrix, pProjectionMatrix);
		pRenderer->Render(deltaTime);
	}

	m_pEngineImGuiContext->SwitchCurrentContext();
	m_pEngineImGuiContext->Update();

	for (std::unique_ptr<engine::Renderer>& pRenderer : m_pEngineRenderers)
	{
		const float* pViewMatrix = nullptr;
		const float* pProjectionMatrix = nullptr;
		if (engine::Camera* pCamera = pRenderer->GetCamera())
		{
			pCamera->Update();
			pViewMatrix = pCamera->GetViewMatrix();
			pProjectionMatrix = pCamera->GetProjectionMatrix();
		}

		pRenderer->UpdateView(pViewMatrix, pProjectionMatrix);
		pRenderer->Render(deltaTime);
	}

	m_pRenderContext->EndFrame();
	
	return !GetMainWindow()->ShouldClose();
}

}