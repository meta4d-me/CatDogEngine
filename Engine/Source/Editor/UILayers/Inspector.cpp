#include "Inspector.h"

#include <imgui/imgui.h>

namespace editor
{

Inspector::~Inspector()
{

}

void Inspector::Init()
{

}

void Inspector::Update()
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