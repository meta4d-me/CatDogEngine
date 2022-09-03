#include "Application.h"
#include "Rendering/UIRenderer.h"
#include "Rendering/SceneRenderer.h"
#include "Rendering/SkyRenderer.h"
#include "Rendering/RenderContext.h"
#include "Utility/Clock.h"
//#include "Windowing/PlatformWindow.h"

// ThirdParty
#include <bgfx/platform.h>
 
namespace engine
{

Application::Application(std::unique_ptr<IGame> pGame) :
	m_pGame(std::move(pGame))
{
}

Application::~Application()
{
	m_pGame->Shutdown();

	delete m_pRenderContext;
	delete m_pUIRenderer;
	delete m_pSceneRenderer;
	delete m_pSkyRenderer;

	//if(m_pWindow != nullptr)
	//{
	//	delete m_pWindow;
	//	m_pWindow = nullptr;
	//}
}

void Application::Init(void* pWindowHandle)
{
	//if(nullptr == pWindowHandle)
	//{
	//	WindowCreateDescriptor windowCreateDescriptor;
	//	windowCreateDescriptor.title = nullptr;
	//	windowCreateDescriptor.width = 1280;
	//	windowCreateDescriptor.height = 720;
	//	m_pWindow = new PlatformWindow(windowCreateDescriptor);
	//
	//	pWindowHandle = m_pWindow->GetNativeWindow();
	//}

	m_pRenderContext = new Rendering::RenderContext(1280, 720, pWindowHandle);
	m_pRenderContext->Init();

	m_pSkyRenderer = new Rendering::SkyRenderer(m_pRenderContext);
	m_pSkyRenderer->Init();
	m_pSkyRenderer->SetViewID(0);

	m_pSceneRenderer = new Rendering::SceneRenderer(m_pRenderContext);
	m_pSceneRenderer->Init();
	m_pSceneRenderer->SetViewID(1);

	m_pUIRenderer = new Rendering::UIRenderer(m_pRenderContext);
	m_pUIRenderer->Init();
	m_pUIRenderer->SetViewID(2);

	m_pGame->Init();
}

bool Application::ShouldClose()
{
	//if(m_pWindow)
	//{
	//	return m_pWindow->ShouldClose();
	//}

	return false;
}

void Application::Execute()
{
	Tools::Clock clock;
	while (!ShouldClose())
	{
		clock.Update();

		//if(m_pWindow)
		//{
		//	m_pWindow->Update();
		//}

		m_pGame->Update(clock.GetDeltaTime());

		m_pRenderContext->BeginFrame();
		//m_pSceneRenderer->Render(clock.GetDeltaTime());
		m_pSkyRenderer->Render(clock.GetDeltaTime());
		//m_pUIRenderer->Render(clock.GetDeltaTime());
		m_pRenderContext->EndFrame();
	}
}

void Application::SetCurrentSceneDatabase(std::string filePath)
{
	m_pSceneRenderer->LoadSceneData(std::move(filePath));
}

}