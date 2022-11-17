#include "EditorImGuiContext.h"

#include <imgui/imgui.h>

namespace editor
{

EditorImGuiContext::EditorImGuiContext()
{
	m_pImGuiContext = ImGui::CreateContext();

	// Doug Binks' darl color scheme
	// https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9
	// ImGuiStyle& style = ImGui::GetStyle();
	// style.FrameRounding = 4.0f;
	// style.WindowBorderSize = 0.0f;
	// ImGui::StyleColorsDark(&style);
}

EditorImGuiContext::~EditorImGuiContext()
{
	ImGui::DestroyContext(m_pImGuiContext);
}

void EditorImGuiContext::Update()
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddMousePosEvent(static_cast<float>(m_mouseX), static_cast<float>(m_mouseY));
	io.AddMouseButtonEvent(ImGuiMouseButton_Left, m_bLeftButtonDown);
	io.AddMouseButtonEvent(ImGuiMouseButton_Right, m_bRightButtonDown);
	io.AddMouseButtonEvent(ImGuiMouseButton_Middle, m_bMiddleButtonDown);
	// Normal mouse devices don't have scoll x.
	io.AddMouseWheelEvent(0.0f, m_mouseScollY);

	ImGui::NewFrame();
	ImGui::SetNextWindowPos(ImVec2(10.0f, 50.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300.0f, 210.0f), ImGuiCond_FirstUseEver);

	ImGui::Begin("Test", 0, 0);
	ImGui::ShowStyleEditor();
	ImGui::End();
}

}