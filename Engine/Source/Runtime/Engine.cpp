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

Engine::Engine()
{
}

Engine::~Engine()
{
	Shutdown();
}

void Engine::Init()
{
	m_pRenderContext = std::make_unique<RenderContext>();
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

		std::unique_ptr<SkyRenderer> pSkyRenderer = std::make_unique<SkyRenderer>(m_pRenderContext->CreateView(), pSwapChain, m_pRenderContext->GetGBuffer());
		std::unique_ptr<SceneRenderer> pSceneRenderer = std::make_unique<SceneRenderer>(m_pRenderContext->CreateView(), pSwapChain, m_pRenderContext->GetGBuffer());
		pSceneRenderer->SetDependentRender(pSkyRenderer.get());

		m_pRenderers.emplace_back(std::move(pSkyRenderer));
		m_pRenderers.emplace_back(std::move(pSceneRenderer));
		m_pRenderers.push_back(std::make_unique<PostProcessRenderer>(m_pRenderContext->CreateView(), pSwapChain, m_pRenderContext->GetGBuffer()));
		for (std::unique_ptr<Renderer>& pRenderer : m_pRenderers)
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
			for (std::unique_ptr<Renderer>& pRenderer : m_pRenderers)
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

}

void Engine::InitCSharpBridge()
{
	m_pCSharpBridge = std::make_unique<CSharpBridge>();
}

void Engine::InitPlatformWindow(const char* pTitle, uint16_t width, uint16_t height)
{
	m_pFlybyCamera = std::make_unique<FlybyCamera>(bx::Vec3(0.0f, 0.0f, -50.0f));
	m_pCameraController = std::make_unique<FirstPersonCameraController>(m_pFlybyCamera.get(), 0.8f /* Mouse Sensitivity */, 20.0f /* Movement Speed*/);
	m_pPlatformWindow = std::make_unique<PlatformWindow>(pTitle, width, height, m_pCameraController.get());
}

}