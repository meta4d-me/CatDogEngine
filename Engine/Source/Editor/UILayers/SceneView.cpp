#include "SceneView.h"

#include "EditorApp.h"
#include "Rendering/GBuffer.h"
#include "Rendering/PBRSkyRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SceneRenderer.h"

#include <imgui/imgui.h>

namespace
{

}

namespace editor
{

SceneView::~SceneView()
{

}

void SceneView::Init()
{
	uint16_t m_width = 800;
	uint16_t m_height = 800;

	engine::RenderContext* pRenderContext = m_pEditorApp->GetRenderContext();
	assert(pRenderContext && "RenderContext should be initilized at first.");

	pRenderContext->InitGBuffer(m_width, m_height);
	pRenderContext->AddRenderer(std::make_unique<engine::PBRSkyRenderer>(pRenderContext, pRenderContext->CreateView(), nullptr));
	pRenderContext->AddRenderer(std::make_unique<engine::SceneRenderer>(pRenderContext, pRenderContext->CreateView(), nullptr));
}

void SceneView::OnResize()
{
	ImVec2 regionSize = ImGui::GetContentRegionAvail();
	uint16_t regionWidth = static_cast<uint16_t>(regionSize.x);
	uint16_t regionHeight = static_cast<uint16_t>(regionSize.y);
	m_pEditorApp->GetRenderContext()->GetGBuffer()->Resize(regionWidth, regionHeight);
}

void SceneView::Update()
{
	const engine::GBuffer* pRenderTarget = m_pEditorApp->GetRenderContext()->GetGBuffer();
	//ImGui::SetNextWindowSize(ImVec2(pRenderTarget->GetWidth(), pRenderTarget->GetHeight()), ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGui::Begin(GetName(), &m_isEnable, flags))
	{
		OnResize();

		const bgfx::FrameBufferHandle* pFrameBufferHandle = pRenderTarget->GetFrameBuffer();
		ImGui::Image(ImTextureID(pRenderTarget->GetBackBufferTextureHandle(0).idx),
			ImVec2(pRenderTarget->GetWidth(), pRenderTarget->GetHeight()));

		ImGui::PopStyleVar();
		ImGui::End();
	}
}

}