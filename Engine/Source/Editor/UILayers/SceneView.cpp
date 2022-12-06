#include "SceneView.h"

#include "Display/Camera.h"
#include "EditorApp.h"
#include "IconFont/IconsMaterialDesignIcons.h"
#include "Rendering/EditorZmoRenderer.h"
#include "Rendering/GBuffer.h"
#include "Rendering/PBRSkyRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SceneRenderer.h"

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <imguizmo/ImGuizmo.h>

namespace
{

struct ImGuizmoOperationMode
{
	// icon font is 16 bits
	const char8_t* pIconFontName;
	const char* pToolStripName;
	ImGuizmo::OPERATION operation;
	bool createUIVerticalLine;
};

// Be careful to control the last ImGuizmoOperationMode object's createUIVerticalLine flag.
// It depends on what the next UI element is.
constexpr ImGuizmoOperationMode OperationModes[] = {
	{ ICON_MDI_CURSOR_DEFAULT, "Select",  ImGuizmo::OPERATION::TRANSLATE_Z, true},
	{ ICON_MDI_ARROW_ALL, "Translate",  ImGuizmo::OPERATION::TRANSLATE, false},
	{ ICON_MDI_ROTATE_ORBIT, "Rotate",  ImGuizmo::OPERATION::ROTATE, false},
	{ ICON_MDI_ARROW_EXPAND_ALL, "Scale",  ImGuizmo::OPERATION::SCALE, false},

	// Universal mode is a combination of Translate/Rotate/Scale.
	// You can do these three operations together.
	{ ICON_MDI_CROP_ROTATE, "Universal",  ImGuizmo::OPERATION::UNIVERSAL, true},
};

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
	//pRenderContext->AddRenderer(std::make_unique<EditorZmoRenderer>(pRenderContext, pRenderContext->CreateView(), nullptr));
	pRenderContext->AddRenderer(std::make_unique<engine::SceneRenderer>(pRenderContext, pRenderContext->CreateView(), nullptr));
}

void SceneView::OnResize()
{
	ImVec2 regionSize = ImGui::GetContentRegionAvail();
	uint16_t regionWidth = static_cast<uint16_t>(regionSize.x);
	uint16_t regionHeight = static_cast<uint16_t>(regionSize.y);
	m_pEditorApp->GetRenderContext()->GetGBuffer()->Resize(regionWidth, regionHeight);
}

void SceneView::UpdateToolMenuButtons()
{
	ImGui::Indent();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	bool isSelected = false;
	for (const auto& operationMode : OperationModes)
	{
		isSelected = operationMode.operation == m_currentOperation;
		if (isSelected)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));
		}
		
		ImGui::SameLine();
		if (ImGui::Button(reinterpret_cast<const char*>(operationMode.pIconFontName)))
		{
			m_currentOperation = operationMode.operation;
		}

		if (isSelected)
		{
			ImGui::PopStyleColor();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(operationMode.pToolStripName);
			ImGui::EndTooltip();
		}
		ImGui::PopStyleVar();

		if (operationMode.createUIVerticalLine)
		{
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
		}
	}

	if (ImGui::Button(reinterpret_cast<const char*>("Options " ICON_MDI_CHEVRON_DOWN)))
	{
		ImGui::OpenPopup("GizmosPopup");
	}

	if (ImGui::BeginPopup("GizmosPopup"))
	{
		ImGui::Checkbox("Option 1", &m_option1);
		ImGui::Checkbox("Option 2", &m_option2);

		ImGui::EndPopup();
	}

	ImGui::SameLine();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SameLine();

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_AXIS_ARROW " 3D")))
	{
	}

	ImGui::SameLine();

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_ANGLE_RIGHT " 2D")))
	{
	}

	ImGui::PopStyleColor();
}

void SceneView::Update()
{
	const engine::GBuffer* pRenderTarget = m_pEditorApp->GetRenderContext()->GetGBuffer();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGui::Begin(GetName(), &m_isEnable, flags))
	{
		// Swicth ImGuizmo operation mode
		UpdateToolMenuButtons();

		ImVec2 cursorPosition = ImGui::GetCursorPos();
		ImVec2 sceneViewSize = ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin() - cursorPosition * 0.5f;
		ImVec2 sceneViewPosition = ImGui::GetWindowPos() + cursorPosition;

		// Inside the implementation, it will check if width or height changes.
		OnResize();

		const bgfx::FrameBufferHandle* pFrameBufferHandle = pRenderTarget->GetFrameBuffer();
		ImGui::Image(ImTextureID(pRenderTarget->GetBackBufferTextureHandle(0).idx),
			ImVec2(pRenderTarget->GetWidth(), pRenderTarget->GetHeight()));

		ImGui::PopStyleVar();
		ImGui::End();
	}

	//ImGui::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);
	//if (ImGuizmo::IsUsing())
	//{
	//	ImGui::Text("Using gizmo");
	//}
	//else
	//{
	//	ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
	//	ImGui::SameLine();
	//	ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
	//	ImGui::SameLine();
	//	ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
	//	ImGui::SameLine();
	//	ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
	//}
}

}