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
	if (ImGui::Begin(GetName(), &m_isEnable, flags))
	{
		//if (ImGui::Image())
		//{
		//
		//}
	
		ImGui::End();
	}
}

}