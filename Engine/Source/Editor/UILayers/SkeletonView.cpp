#include "SkeletonView.h"

#include "ECWorld/SceneWorld.h"
#include <imgui/imgui.h>
#include <ImGui/IconFont/IconsMaterialDesignIcons.h>

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

void SkeletonView::DrawBone(cd::SceneDatabase* pSceneDatabase,cd::Bone* pBone)
{
    if(ImGui::TreeNode(pBone->GetName()))
    {
        for (auto& child : pBone->GetChildIDs())
        {
           // cd::Bone bone = pSceneDatabase->GetBone(child.Data());
           // if (ImGui::TreeNode((void*)(intptr_t)i, "Child %d", i))
           // {
           //     ImGui::Text(reinterpret_cast<const char*>(ICON_MDI_BONE));
           //     
           //     ImGui::SameLine();
           //     ImGui::Selectable("blah blah");                         
           //     ImGui::TreePop();
           // }
        }

        ImGui::TreePop();
    }
}

void SkeletonView::DrawSkeleton(engine::SceneWorld* pSceneWorld, cd::Bone* root)
{
    ImGui::Begin("Skeleton");
    cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();
   // DrawBone(root);

    ImGui::End();
}

void SkeletonView::Update()
{
    auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin(GetName(), &m_isEnable, flags);
   // DrawBone();

    ImGui::End();
}

}