#include "EditorApp.h"

#include "Application/Engine.h"
#include "Display/FlybyCamera.h"
#include "Display/FirstPersonCameraController.h"
#include "ECWorld/EditorSceneWorld.h"
#include "ImGui/EditorImGuiViewport.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/UILayers/DebugPanel.h"
#include "Rendering/ImGuiRenderer.h"
#include "Rendering/PBRSkyRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/WorldRenderer.h"
#include "Resources/ResourceBuilder.h"
#include "Scene/SceneDatabase.h"
#include "UILayers/AssetBrowser.h"
#include "UILayers/EntityList.h"
#include "UILayers/GameView.h"
#include "UILayers/ImGuizmoView.h"
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

	InitECWorld();

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

	m_pEditorImGuiContext = std::make_unique<engine::ImGuiContextInstance>(GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight(), true/*dockable*/);

	// TODO : more font files to load and switch dynamicly.
	std::vector<std::string> ttfFileNames = { "FanWunMing-SB.ttf" };
	m_pEditorImGuiContext->LoadFontFiles(ttfFileNames, language);

	// Bind event callbacks from current available input devices.
	GetMainWindow()->OnResize.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnResize>(m_pEditorImGuiContext.get());

	// Set style settings.
	m_pEditorImGuiContext->SetImGuiThemeColor(engine::ThemeColor::Dark);

	ImGui::GetIO().BackendRendererUserData = m_pRenderContext.get();

	// Add UI layers after finish imgui and rendering contexts' initialization.
	m_pEditorImGuiContext->AddStaticLayer(std::make_unique<MainMenu>("MainMenu"));

	auto pEntityList = std::make_unique<EntityList>("EntityList");
	pEntityList->SetSceneWorld(m_pEditorSceneWorld.get());
	m_pEditorImGuiContext->AddDynamicLayer(cd::MoveTemp(pEntityList));

	//m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<GameView>("GameView"));

	auto pSceneView = std::make_unique<SceneView>("SceneView");
	pSceneView->SetSceneWorld(m_pEditorSceneWorld.get());
	m_pSceneView = pSceneView.get();
	m_pEditorImGuiContext->AddDynamicLayer(cd::MoveTemp(pSceneView));
	m_pEditorImGuiContext->AddDynamicLayer(std::make_unique<Inspector>("Inspector"));

	auto pAssetBrowser = std::make_unique<AssetBrowser>("AssetBrowser");
	pAssetBrowser->SetSceneRenderer(m_pSceneRenderer);
	pAssetBrowser->SetSceneWorld(m_pEditorSceneWorld.get());
	GetMainWindow()->OnDropFile.Bind<editor::AssetBrowser, &editor::AssetBrowser::ImportAssetFile>(pAssetBrowser.get());

	m_pEditorImGuiContext->AddDynamicLayer(cd::MoveTemp(pAssetBrowser));
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

	//m_pEngineImGuiContext->AddDynamicLayer(std::make_unique<engine::DebugPanel>("DebugPanel"));
	//m_pEngineImGuiContext->AddDynamicLayer(std::make_unique<editor::ImGuizmoView>("ImGuizmoView"));

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

void EditorApp::InitECWorld()
{
	m_pEditorSceneWorld = std::make_unique<EditorSceneWorld>();
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
	m_pCamera = std::make_unique<engine::FlybyCamera>(cd::Point(0.0f, 0.0f, -50.0f));
	m_pCamera->SetAspect(1.0f);
	m_pCamera->SetFov(45.0f);
	m_pCamera->SetNearPlane(0.1f);
	m_pCamera->SetFarPlane(1000.0f);
	m_pCamera->SetHomogeneousNdc(bgfx::getCaps()->homogeneousDepth);
	m_pRenderContext->SetCamera(m_pCamera.get());

	m_pCameraController = std::make_unique<engine::FirstPersonCameraController>(m_pCamera.get(), 50.0f /* Mouse Sensitivity */, 160.0f /* Movement Speed*/);

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

	auto pSkyRenderer = std::make_unique<engine::PBRSkyRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pSceneRenderTarget);
	pSkyRenderer->SetEntity(m_pEditorSceneWorld->GetSkyEntity());
	AddEngineRenderer(cd::MoveTemp(pSkyRenderer));

	auto pSceneRenderer = std::make_unique<engine::WorldRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pSceneRenderTarget);
	m_pSceneRenderer = pSceneRenderer.get();
	pSceneRenderer->SetWorld(m_pEditorSceneWorld->GetWorld());
	pSceneRenderer->SetMeshEntites(&m_pEditorSceneWorld->GetMeshEntites());
	AddEngineRenderer(cd::MoveTemp(pSceneRenderer));

	// Note that if you don't want to use ImGuiRenderer for engine, you should also disable EngineImGuiContext.
	AddEngineRenderer(std::make_unique<engine::ImGuiRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pSceneRenderTarget));
}

void EditorApp::AddEditorRenderer(std::unique_ptr<engine::Renderer> pRenderer)
{
	pRenderer->Init();
	m_pEditorRenderers.emplace_back(cd::MoveTemp(pRenderer));
}

void EditorApp::AddEngineRenderer(std::unique_ptr<engine::Renderer> pRenderer)
{
	pRenderer->Init();
	pRenderer->SetCamera(m_pCamera.get());
	m_pEngineRenderers.emplace_back(cd::MoveTemp(pRenderer));
}

bool EditorApp::Update(float deltaTime)
{
	GetMainWindow()->Update();

	m_pCameraController->Update(deltaTime);

	m_pEditorImGuiContext->Update(deltaTime);
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

	m_pEngineImGuiContext->SetWindowPosOffset(m_pSceneView->GetWindowPosX(), m_pSceneView->GetWindowPosY());
	m_pEngineImGuiContext->Update(deltaTime);
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