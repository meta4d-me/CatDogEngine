#include "ImGuizmoView.h"

#include "Display/Camera.h"
#include "Rendering/RenderContext.h"

#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>

namespace editor
{

ImGuizmoView::~ImGuizmoView()
{

}

void ImGuizmoView::Init()
{

}

void ImGuizmoView::Update()
{
	ImGuiIO& io = ImGui::GetIO();
	engine::RenderContext* pCurrentRenderContext = reinterpret_cast<engine::RenderContext*>(io.BackendRendererUserData);
	engine::Camera* pCamera = pCurrentRenderContext->GetCamera();

	bool isPerspective = true;
	ImGuizmo::SetOrthographic(!isPerspective);
	ImGuizmo::BeginFrame();

	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	static const float identityMatrix[16] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };
	ImGuizmo::DrawGrid(pCamera->GetViewMatrix().Begin(), pCamera->GetProjectionMatrix().Begin(), identityMatrix, 100.f);
}

}