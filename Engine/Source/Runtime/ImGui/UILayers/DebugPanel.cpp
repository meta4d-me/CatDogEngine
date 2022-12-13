#include "DebugPanel.h"

#include <imgui/imgui.h>

namespace engine
{

DebugPanel::~DebugPanel()
{

}

void DebugPanel::Init()
{

}

void DebugPanel::Update()
{
	//auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	//ImGui::Begin(GetName(), &m_isEnable, flags);
	//ImGui::End();
	ImGui::ShowDemoWindow();
}

}