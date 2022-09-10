#include "Engine.h"

#include "API/CSharpBridge.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SceneRenderer.h"
#include "Rendering/SkyRenderer.h"
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
	if(m_pPlatformWindow)
	{
		m_pRenderContext = new RenderContext();
		m_pRenderContext->Init(m_pPlatformWindow->GetWidth(), m_pPlatformWindow->GetHeight(), m_pPlatformWindow->GetNativeWindow());

		m_pSceneRenderer = new SceneRenderer(m_pRenderContext);
		m_pSceneRenderer->Init();
		m_pSceneRenderer->SetViewID(0);
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