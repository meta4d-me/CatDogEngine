#include "OutputLog.h"

#include <imgui/imgui.h>

namespace editor
{

OutputLog::~OutputLog()
{

}

void OutputLog::Init()
{

}

void OutputLog::Update()
{
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin(GetName(), &m_isEnable, flags);
	ImGui::End();
}

}