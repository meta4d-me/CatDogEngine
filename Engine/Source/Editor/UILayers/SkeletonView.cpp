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

void SkeletonView::DrawBone(cd::SceneDatabase* pSceneDatabase, const cd::Bone& Bone)
{
    if (Bone.GetChildIDs().empty())
    {
        ImGui::Selectable(Bone.GetName());
        ImGui::SameLine();
        ImGui::Text(reinterpret_cast<const char*>(ICON_MDI_BONE));
        return;
    }

    bool isOpen = ImGui::TreeNode(Bone.GetName());
    ImGui::SameLine();
    ImGui::Text(reinterpret_cast<const char*>(ICON_MDI_BONE));
    if(isOpen)
    {
        for (auto& child : Bone.GetChildIDs())
        {
            const cd::Bone& bone = pSceneDatabase->GetBone(child.Data());
            DrawBone(pSceneDatabase, bone);
        }

        ImGui::TreePop();
    }
}

void SkeletonView::DrawSkeleton(engine::SceneWorld* pSceneWorld)
{
    cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();
    if (0 == pSceneDatabase->GetSkeletonCount())
    {
        return;
    }
    for (uint32_t skeletonIndex = 0; skeletonIndex < pSceneDatabase->GetSkeletonCount(); skeletonIndex++)
    {
        const cd::Skeleton& skeleton = pSceneDatabase->GetSkeleton(skeletonIndex);
        auto rootBoneID = skeleton.GetRootBoneID();
        const cd::Bone& rootBone = pSceneDatabase->GetBone(rootBoneID.Data());
        DrawBone(pSceneDatabase, rootBone);
    }
}

void SkeletonView::Update()
{
    constexpr auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin(GetName(), &m_isEnable, flags);
    engine::SceneWorld* pSceneWorld = GetSceneWorld();
    engine::Entity selectedEntity = pSceneWorld->GetSelectedEntity();
    if (engine::INVALID_ENTITY == selectedEntity)
    {
        ImGui::End();
        return;
    }
    DrawSkeleton(pSceneWorld);

    ImGui::End();
}

}