#include "Engine.h"

#include "API/CSharpBridge.h"
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
	// If engine already set up an OS's target native window, then it should be game mode with only one swap chain.
	// If not, it should be editor mode with multiple swap chains binding with different views.
	if(m_pPlatformWindow)
	{
		m_pRenderContext = new RenderContext();
		m_pRenderContext->Init();

		uint8_t swapChainID = m_pRenderContext->CreateSwapChain(m_pPlatformWindow->GetNativeWindow(), m_pPlatformWindow->GetWidth(), m_pPlatformWindow->GetHeight());
		SwapChain* pSwapChain = m_pRenderContext->GetSwapChain(swapChainID);
		
		m_pSceneRenderer = new SceneRenderer(m_pRenderContext->CreateView(), pSwapChain);
		m_pSceneRenderer->Init();

		//m_pSkyRenderer = new SkyRenderer(m_pRenderContext->CreateView(), pSwapChain);
		//m_pSkyRenderer->Init();
		//
		//m_pUIRenderer = new UIRenderer(m_pRenderContext->CreateView(), pSwapChain);
		//m_pUIRenderer->Init();
	}
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
			m_pSceneRenderer->Render(clock.GetDeltaTime());
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
}

void Engine::InitCSharpBridge()
{
	m_pCSharpBridge = new CSharpBridge();
}

void Engine::InitPlatformWindow(const char* pTitle, uint16_t width, uint16_t height)
{
	m_pPlatformWindow = new PlatformWindow(pTitle, width, height);
}

}