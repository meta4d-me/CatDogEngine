#include "EditorZmoRenderer.h"

#include "Rendering/GBuffer.h"
#include "Rendering/RenderContext.h"

#include <bx/math.h>
#include <imgui/imgui.h>
#include <imguizmo/imguizmo.h>

namespace editor
{

void EditorZmoRenderer::Init()
{
}

EditorZmoRenderer::~EditorZmoRenderer()
{
}

void EditorZmoRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bool isPerspective = true;
	ImGuizmo::SetOrthographic(!isPerspective);
	ImGuizmo::BeginFrame();

	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	static const float identityMatrix[16] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };
	ImGuizmo::DrawGrid(pViewMatrix, pProjectionMatrix, identityMatrix, 100.f);
}

void EditorZmoRenderer::Render(float deltaTime)
{
}

}