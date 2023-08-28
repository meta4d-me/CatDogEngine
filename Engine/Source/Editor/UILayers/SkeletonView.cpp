#include "SkeletonView.h"

#include "ECWorld/SceneWorld.h"
#include <imgui/imgui.h>

namespace editor
{
SkeletonView::~SkeletonView()
{

}
void SkeletonView::Init()
{
   
}
void SkeletonView::SkeletonWidow()
{
	if (m_isSkeletonWidowOpen)
	{
        ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        bool showDetachedWindow = true;
        bool isDetachedWindowDragged;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;
        if (!showDetachedWindow)
            windowFlags |= ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("Detached Window", &showDetachedWindow, windowFlags);

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            isDetachedWindowDragged = true;

        if (isDetachedWindowDragged)
            ImGui::SetWindowPos(ImGui::GetMousePos());

        ImGui::Text("Drag me outside the main window!");

        if (!isDetachedWindowDragged && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            isDetachedWindowDragged = ImGui::IsWindowHovered();
	}
}

void SkeletonView::Update()
{
    auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin(GetName(), &m_isEnable, flags);
    engine::SceneWorld* pSceneWorld = GetSceneWorld();
    engine::Entity selectedEntity = pSceneWorld->GetSelectedEntity();
   
    if (engine::INVALID_ENTITY == selectedEntity)
    {
        ImGui::End();
        return;
    }

    ImGui::BeginChild("SkeletonView");
    ImGui::Text("Skeleton");
    ImGui::EndChild();

    ImGui::End();
}

}