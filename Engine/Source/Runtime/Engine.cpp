#include "Engine.h"

#include "API/CSharpBridge.h"
#include "FirstPersonCameraController.h"
#include "FlybyCamera.h"
#include "Rendering/GBuffer.h"
#include "Rendering/PostProcessRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SceneRenderer.h"
#include "Rendering/SkyRenderer.h"
#include "Rendering/SwapChain.h"
#include "Rendering/UIRenderer.h"
#include "Utility/Clock.h"
#include "Windowing/PlatformWindow.h"

namespace engine
{

Engine::~Engine()
{
	Shutdown();
}

void Engine::Init()
{
	m_pRenderContext = new RenderContext();
	m_pRenderContext->Init();

	// If engine already set up an OS's target native window, then it should be game mode with only one swap chain.
	// If not, it should be editor mode with multiple swap chains binding with different views.
	if(m_pPlatformWindow)
	{
		uint16_t width = m_pPlatformWindow->GetWidth();
		uint16_t height = m_pPlatformWindow->GetHeight();

		// Initialize Camera
		if (m_pFlybyCamera)
		{
			m_pFlybyCamera->SetAspect(static_cast<float>(width) / height);
			m_pFlybyCamera->SetFov(45.0f);
			m_pFlybyCamera->SetNearPlane(0.1f);
			m_pFlybyCamera->SetFarPlane(1000.0f);
		}
		
		uint8_t swapChainID = m_pRenderContext->CreateSwapChain(m_pPlatformWindow->GetNativeWindow(), width, height);
		SwapChain* pSwapChain = m_pRenderContext->GetSwapChain(swapChainID);
		m_pRenderContext->InitGBuffer(width, height);

		SkyRenderer* pSkyRenderer = new SkyRenderer(m_pRenderContext->CreateView(), pSwapChain, m_pRenderContext->GetGBuffer());
		m_pRenderers.push_back(pSkyRenderer);
		SceneRenderer* pSceneRenderer = new SceneRenderer(m_pRenderContext->CreateView(), pSwapChain, m_pRenderContext->GetGBuffer());
		m_pRenderers.push_back(pSceneRenderer);
		pSceneRenderer->SetDependentRender(pSkyRenderer);

		m_pRenderers.push_back(new PostProcessRenderer(m_pRenderContext->CreateView(), pSwapChain, m_pRenderContext->GetGBuffer()));
		for (Renderer* pRenderer : m_pRenderers)
		{
			pRenderer->Init();
		}
	}
}

void Engine::MainLoop()
{
	Tools::Clock clock;

	while (true)
	{
		clock.Update();

		if (m_pCameraController) 
		{
			m_pCameraController->Update(clock.GetDeltaTime());
		}

		if (m_pPlatformWindow && m_pFlybyCamera)
		{
			// Update in case of resize happened
			m_pFlybyCamera->SetAspect(static_cast<float>(m_pPlatformWindow->GetWidth()) / m_pPlatformWindow->GetHeight());
			m_pFlybyCamera->Update();

			m_pPlatformWindow->Update();
			if(m_pPlatformWindow->ShouldClose())
			{
				break;
			}

			m_pRenderContext->BeginFrame();
			for (Renderer* pRenderer : m_pRenderers)
			{
				pRenderer->UpdateView(m_pFlybyCamera->GetViewMatrix(), m_pFlybyCamera->GetProjectionMatrix());
				pRenderer->Render(clock.GetDeltaTime());
			}
			m_pRenderContext->EndFrame();
		}
	}
}

void Engine::Shutdown()
{
	if (m_pCSharpBridge)
	{
		delete m_pCSharpBridge;
		m_pCSharpBridge = nullptr;
	}

	if (m_pPlatformWindow)
	{
		delete m_pPlatformWindow;
		m_pPlatformWindow = nullptr;
	}

	if (m_pFlybyCamera)
	{
		delete m_pFlybyCamera;
		m_pFlybyCamera = nullptr;
	}

	if (m_pCameraController)
	{
		delete m_pCameraController;
		m_pCameraController = nullptr;
	}
}

void Engine::InitCSharpBridge()
{
	m_pCSharpBridge = new CSharpBridge();
}

void Engine::InitPlatformWindow(const char* pTitle, uint16_t width, uint16_t height)
{
	m_pFlybyCamera = new FlybyCamera(bx::Vec3(0.0f, 0.0f, -50.0f));
	m_pCameraController = new FirstPersonCameraController(
		m_pFlybyCamera, 
		0.8f /* Mouse Sensitivity */, 
		20.0f /* Movement Speed*/);
	m_pPlatformWindow = new PlatformWindow(pTitle, width, height, m_pCameraController);
}

}