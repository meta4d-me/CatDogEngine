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
	ImGui::Begin(GetName(), &m_isEnable, flags);
	ImGui::End();
}

}