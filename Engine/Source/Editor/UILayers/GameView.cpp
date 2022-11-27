#include "GameView.h"

#include "EditorApp.h"
#include "Rendering/GBuffer.h"
#include "Rendering/PBRSkyRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/SceneRenderer.h"

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
	if (ImGui::Begin("GameView", &m_isEnable, flags))
	{
		//if (ImGui::Image())
		//{
		//
		//}

		ImGui::End();
	}
}

}