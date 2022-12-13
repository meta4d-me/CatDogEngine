#include "EntityList.h"

#include <imgui/imgui.h>

namespace editor
{

EntityList::~EntityList()
{

}

void EntityList::Init()
{

}

void EntityList::Update()
{
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin(GetName(), &m_isEnable, flags);
	ImGui::End();
}

}