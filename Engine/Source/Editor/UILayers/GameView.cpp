#include "GameView.h"

#include <imgui/imgui.h>

namespace
{

}

namespace editor
{

GameView::~GameView()
{

}

void GameView::Init()
{

}

void GameView::OnResize()
{
}

void GameView::Update()
{
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin(GetName(), &m_isEnable, flags);
	ImGui::End();
}

}