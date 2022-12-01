#include "AssetBrowser.h"

#include <imgui/imgui.h>

namespace editor
{

AssetBrowser::~AssetBrowser()
{

}

void AssetBrowser::Init()
{

}

void AssetBrowser::Update()
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