#include "EditorApp.h"

#include "Application/Engine.h"
#include "EditorRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SwapChain.h"
#include "Window/Window.h"

#include <imgui/imgui.h>

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
	m_pMainWindow = std::make_unique<engine::Window>(initArgs.pTitle, width, height);

	m_pEditorImGuiContext = std::make_unique<EditorImGuiContext>();
	m_pMainWindow->OnMouseLBDown.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseLBDown>(m_pEditorImGuiContext.get());
	m_pMainWindow->OnMouseLBUp.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseLBUp>(m_pEditorImGuiContext.get());
	m_pMainWindow->OnMouseRBDown.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseRBDown>(m_pEditorImGuiContext.get());
	m_pMainWindow->OnMouseRBUp.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseRBUp>(m_pEditorImGuiContext.get());
	m_pMainWindow->OnMouseMBDown.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseMBDown>(m_pEditorImGuiContext.get());
	m_pMainWindow->OnMouseMBUp.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseMBUp>(m_pEditorImGuiContext.get());
	m_pMainWindow->OnMouseWheel.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseWheel>(m_pEditorImGuiContext.get());
	m_pMainWindow->OnMouseMove.Bind<EditorImGuiContext, &EditorImGuiContext::OnMouseMove>(m_pEditorImGuiContext.get());

	//m_pMainWindow->OnKeyDown.Bind<EditorImGuiContext, &EditorImGuiContext::OnKeyPress>(m_pEditorImGuiContext.get());
	//m_pMainWindow->OnKeyUp.Bind<EditorImGuiContext, &EditorImGuiContext::OnKeyRelease>(m_pEditorImGuiContext.get());

	m_pRenderContext = m_pEngine->GetRenderContext();
	uint8_t swapChainID = m_pRenderContext->CreateSwapChain(m_pMainWindow->GetNativeWindow(), width, height);
	engine::SwapChain* pSwapChain = m_pRenderContext->GetSwapChain(swapChainID);
	m_pRenderers.push_back(std::make_unique<editor::EditorRenderer>(m_pRenderContext, m_pRenderContext->CreateView(), pSwapChain));
	m_pMainWindow->OnResize.Bind<engine::RenderContext, &engine::RenderContext::ResizeFrameBuffers>(m_pRenderContext);

	for (std::unique_ptr<engine::Renderer>& pRenderer : m_pRenderers)
	{
		pRenderer->Init();
	}
}

void EditorApp::Shutdown()
{
}

bool EditorApp::Update(float deltaTime)
{
	m_pMainWindow->Update();

	m_pEditorImGuiContext->Update();

	m_pRenderContext->BeginFrame();
	for (std::unique_ptr<engine::Renderer>& pRenderer : m_pRenderers)
	{
		pRenderer->UpdateView(nullptr, nullptr);
		pRenderer->Render(deltaTime);
	}
	m_pRenderContext->EndFrame();
	
	return !m_pMainWindow->ShouldClose();
}

}