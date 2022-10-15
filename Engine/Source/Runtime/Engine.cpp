#include "Engine.h"

#include "API/CSharpBridge.h"
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

	// Default values
	uint16_t width = 1200;
	uint16_t height = 900;
	// If engine already set up an OS's target native window, then it should be game mode with only one swap chain.
	// If not, it should be editor mode with multiple swap chains binding with different views.
	if(m_pPlatformWindow)
	{
		width = m_pPlatformWindow->GetWidth();
		height = m_pPlatformWindow->GetHeight();
		uint8_t swapChainID = m_pRenderContext->CreateSwapChain(m_pPlatformWindow->GetNativeWindow(), width, height);
		SwapChain* pSwapChain = m_pRenderContext->GetSwapChain(swapChainID);
		m_pRenderContext->InitGBuffer(width, height);		

		m_pRenderers.push_back(new SkyRenderer(m_pRenderContext->CreateView(), pSwapChain, m_pRenderContext->GetGBuffer()));
		//m_pRenderers.push_back(new SceneRenderer(m_pRenderContext->CreateView(), pSwapChain, m_pRenderContext->GetGBuffer()));
		m_pRenderers.push_back(new PostProcessRenderer(m_pRenderContext->CreateView(), pSwapChain, m_pRenderContext->GetGBuffer()));
		for (Renderer* pRenderer : m_pRenderers)
		{
			pRenderer->Init();
		}
	}

	// Initialize Camera
	if (!m_pFlybyCamera) {
		m_pFlybyCamera = new FlybyCamera(bx::Vec3(0.0f, 30.0f, 0.0f));
	}
	m_pFlybyCamera->Pitch(-30.0f);
}

void Engine::MainLoop()
{
	Tools::Clock clock;

	while (true)
	{
		clock.Update();

		if(m_pPlatformWindow)
		{
			m_pPlatformWindow->Update();
			if(m_pPlatformWindow->ShouldClose())
			{
				break;
			}

			m_pRenderContext->BeginFrame();
			for (Renderer* pRenderer : m_pRenderers)
			{
				pRenderer->UpdateView();
				pRenderer->Render(clock.GetDeltaTime());
			}
			m_pRenderContext->EndFrame();
		}

		if (m_pFlybyCamera)
		{
			m_pFlybyCamera->Update();
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
}

void Engine::InitCSharpBridge()
{
	m_pCSharpBridge = new CSharpBridge();
}

// TODO make this platform generic init
void Engine::InitPlatformWindow(const char* pTitle, uint16_t width, uint16_t height)
{
	m_pPlatformWindow = new PlatformWindow(pTitle, width, height);
}

}