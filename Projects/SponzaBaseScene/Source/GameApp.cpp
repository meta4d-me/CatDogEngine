#include "GameApp.h"

#include "Application/Engine.h"
#include "Display/FirstPersonCameraController.h"
#include "Display/FlybyCamera.h"
#include "Rendering/PBRSkyRenderer.h"
#include "Rendering/PostProcessRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SwapChain.h"
#include "Rendering/SceneRenderer.h"
#include "Rendering/SkyRenderer.h"
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

	// Rendering
	m_pRenderContext = m_pEngine->GetRenderContext();

	// Camera
	m_pFlybyCamera = std::make_unique<engine::FlybyCamera>(bx::Vec3(0.0f, 0.0f, -50.0f));
	m_pFlybyCamera->SetAspect(aspect);
	m_pFlybyCamera->SetFov(45.0f);
	m_pFlybyCamera->SetNearPlane(0.1f);
	m_pFlybyCamera->SetFarPlane(1000.0f);
	m_pFlybyCamera->SetHomogeneousNdc(bgfx::getCaps()->homogeneousDepth);
	m_pRenderContext->SetCamera(m_pFlybyCamera.get());
	m_pMainWindow->OnResize.Bind<engine::Camera, &engine::Camera::SetAspect>(m_pFlybyCamera.get());

	// Camera controller
	m_pCameraController = std::make_unique<engine::FirstPersonCameraController>(m_pFlybyCamera.get(), 50.0f /* Mouse Sensitivity */, 80.0f /* Movement Speed*/);
	m_pMainWindow->OnMouseRBDown.Bind<engine::FirstPersonCameraController, &engine::FirstPersonCameraController::OnMouseRBPress>(m_pCameraController.get());
	m_pMainWindow->OnMouseRBUp.Bind<engine::FirstPersonCameraController, &engine::FirstPersonCameraController::OnMouseRBRelease>(m_pCameraController.get());
	m_pMainWindow->OnMouseMoveRelative.Bind<engine::FirstPersonCameraController, &engine::FirstPersonCameraController::SetMousePosition>(m_pCameraController.get());
	m_pMainWindow->OnKeyDown.Bind<engine::FirstPersonCameraController, &engine::FirstPersonCameraController::OnKeyPress>(m_pCameraController.get());
	m_pMainWindow->OnKeyUp.Bind<engine::FirstPersonCameraController, &engine::FirstPersonCameraController::OnKeyRelease>(m_pCameraController.get());

	// Renderers
	uint8_t swapChainID = m_pRenderContext->CreateSwapChain(m_pMainWindow->GetNativeHandle(), width, height);
	engine::SwapChain* pSwapChain = m_pRenderContext->GetSwapChain(swapChainID);
	m_pRenderContext->InitGBuffer(width, height);
	m_pRenderContext->AddRenderer(std::make_unique<engine::PBRSkyRenderer>(m_pRenderContext, m_pRenderContext->CreateView(), pSwapChain));
	m_pRenderContext->AddRenderer(std::make_unique<engine::SceneRenderer>(m_pRenderContext, m_pRenderContext->CreateView(), pSwapChain));
	m_pRenderContext->AddRenderer(std::make_unique<engine::PostProcessRenderer>(m_pRenderContext, m_pRenderContext->CreateView(), pSwapChain));
	m_pMainWindow->OnResize.Bind<engine::RenderContext, &engine::RenderContext::ResizeFrameBuffers>(m_pRenderContext);
}

void GameApp::Shutdown()
{

}

bool GameApp::Update(float deltaTime)
{
	m_pMainWindow->Update();

	m_pCameraController->Update(deltaTime);
	m_pFlybyCamera->Update();

	m_pRenderContext->Update(deltaTime);

	return !m_pMainWindow->ShouldClose();
}

}