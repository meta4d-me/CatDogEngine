#include "GameApp.h"

#include "Application/Engine.h"
#include "Display/FirstPersonCameraController.h"
#include "Display/FlybyCamera.h"
#include "Framework/Processor.h"
#include "ImGui/ImGuiContextInstance.h"
#include "Producers/CDProducer/CDProducer.h"
#include "Rendering/BgfxConsumer.h"
#include "Rendering/ImGuiRenderer.h"
#include "Rendering/PBRSkyRenderer.h"
#include "Rendering/PostProcessRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/RenderTarget.h"
#include "Rendering/SceneRenderer.h"
#include "Rendering/SkyRenderer.h"
#include "Rendering/TerrainRenderer.h"
#include "ImGui/UILayers/DebugPanel.h"
#include "Window/Window.h"

namespace game
{

GameApp::GameApp()
{

}

GameApp::~GameApp()
{

}

void GameApp::Init(engine::EngineInitArgs initArgs)
{
	uint16_t width = initArgs.width;
	uint16_t height = initArgs.height;
	float aspect = static_cast<float>(width) / height;
	m_pMainWindow = std::make_unique<engine::Window>(initArgs.pTitle, width, height);
	m_pMainWindow->SetWindowIcon(initArgs.pIconFilePath);

	// ImGui
	m_pImGuiContext = std::make_unique<engine::ImGuiContextInstance>(width, height);
	std::vector<std::string> ttfFileNames = { "FanWunMing-SB.ttf" };
	m_pImGuiContext->LoadFontFiles(ttfFileNames, initArgs.language);
	m_pImGuiContext->SetImGuiThemeColor(engine::ThemeColor::Dark);
	m_pImGuiContext->AddDynamicLayer(std::make_unique<engine::DebugPanel>("DebugPanel"));
	m_pMainWindow->OnResize.Bind<engine::ImGuiContextInstance, &engine::ImGuiContextInstance::OnResize>(m_pImGuiContext.get());

	// Rendering
	m_pRenderContext = std::make_unique<engine::RenderContext>();
	m_pRenderContext->Init();

	// Camera
	m_pFlybyCamera = std::make_unique<engine::FlybyCamera>(cd::Point(0.0f, 50.0f, -50.0f));
	m_pFlybyCamera->SetAspect(aspect);
	m_pFlybyCamera->SetFov(45.0f);
	m_pFlybyCamera->SetNearPlane(0.1f);
	m_pFlybyCamera->SetFarPlane(2000.0f);
	m_pFlybyCamera->SetHomogeneousNdc(bgfx::getCaps()->homogeneousDepth);
	m_pRenderContext->SetCamera(m_pFlybyCamera.get());
	m_pMainWindow->OnResize.Bind<engine::Camera, &engine::Camera::SetAspect>(m_pFlybyCamera.get());

	// Camera controller
	m_pCameraController = std::make_unique<engine::FirstPersonCameraController>(
		m_pFlybyCamera.get(), 
		15.0f /* horizontal sensitivity */,
		5.0f /* vertical sensitivity */,
		160.0f /* Movement Speed*/);

	// Renderers
	constexpr engine::StringCrc gameSwapChainName("GameSwapChain");
	engine::RenderTarget* pSwapChainRenderTarget = m_pRenderContext->CreateRenderTarget(gameSwapChainName, width, height, m_pMainWindow->GetNativeHandle());

	constexpr engine::StringCrc mainRenderTargetName("SceneRenderTarget");
	std::vector<engine::AttachmentDescriptor> attachmentDesc = {
		{.textureFormat = engine::TextureFormat::RGBA32F },
		{.textureFormat = engine::TextureFormat::RGBA32F },
		{.textureFormat = engine::TextureFormat::D32F },
	};
	engine::RenderTarget* pMainRenderTarget = m_pRenderContext->CreateRenderTarget(mainRenderTargetName, width, height, std::move(attachmentDesc));

	AddRenderer(std::make_unique<engine::SkyRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pMainRenderTarget));
	auto pSceneRenderer = std::make_unique<engine::SceneRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pMainRenderTarget);

	std::string sceneFilePath = CDENGINE_RESOURCES_ROOT_PATH;
	sceneFilePath += "Models/dragon_shield.cdscene";
	cdtools::CDProducer producer(sceneFilePath.c_str());
	engine::BgfxConsumer consumer("");
	cdtools::Processor processor(&producer, &consumer);
	processor.Run();
	pSceneRenderer->SetRenderDataContext(consumer.GetRenderDataContext());

	AddRenderer(cd::MoveTemp(pSceneRenderer));
	AddRenderer(std::make_unique<engine::TerrainRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pMainRenderTarget));
	AddRenderer(std::make_unique<engine::PostProcessRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pSwapChainRenderTarget));
	AddRenderer(std::make_unique<engine::ImGuiRenderer>(m_pRenderContext.get(), m_pRenderContext->CreateView(), pSwapChainRenderTarget));
	m_pMainWindow->OnResize.Bind<engine::RenderContext, &engine::RenderContext::OnResize>(m_pRenderContext.get());

	m_pFlybyCamera->FrameAll(processor.GetSceneDatabase()->GetAABB());
}

void GameApp::Shutdown()
{

}

void GameApp::AddRenderer(std::unique_ptr<engine::Renderer> pRenderer)
{
	pRenderer->Init();
	pRenderer->SetCamera(m_pFlybyCamera.get());
	m_pRenderers.emplace_back(std::move(pRenderer));
}

bool GameApp::Update(float deltaTime)
{
	m_pMainWindow->Update();

	m_pImGuiContext->Update(deltaTime);

	m_pCameraController->Update(deltaTime);

	m_pRenderContext->BeginFrame();

	for (std::unique_ptr<engine::Renderer>& pRenderer : m_pRenderers)
	{
		const float* pViewMatrix = nullptr;
		const float* pProjectionMatrix = nullptr;
		if (engine::Camera* pCamera = pRenderer->GetCamera())
		{
			pViewMatrix = pCamera->GetViewMatrix().Begin();
			pProjectionMatrix = pCamera->GetProjectionMatrix().Begin();
		}

		pRenderer->UpdateView(pViewMatrix, pProjectionMatrix);
		pRenderer->Render(deltaTime);
	}

	m_pRenderContext->EndFrame();

	return !m_pMainWindow->ShouldClose();
}

}