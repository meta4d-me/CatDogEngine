#include "SkeletonView.h"

#include "Display/CameraController.h"
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

void SkeletonView::DrawBone(engine::SceneWorld* pSceneWorld, const cd::Bone& Bone)
{
    cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();
    if (Bone.GetChildIDs().empty())
    {
        ImGui::Selectable(Bone.GetName());
        ImGui::SameLine();
        ImGui::Text(reinterpret_cast<const char*>(ICON_MDI_BONE));
        return;
    }

    bool isOpen = ImGui::TreeNodeEx(Bone.GetName(), ImGuiTreeNodeFlags_OpenOnArrow);
    if (ImGui::IsItemClicked())
    {
        if (ImGui::IsMouseDoubleClicked(0))
        {
            pSceneWorld->SetSelectedBoneID(Bone.GetID());
            if (auto* pSkinMeshComponent = pSceneWorld->GetSkinMeshComponent(pSceneWorld->GetSelectedEntity()))
            {
                if (m_pCameraController)
                {
                    const cd::Vec3f& position = pSkinMeshComponent->GetBoneMatrix(pSceneWorld->GetSelectedBoneID().Data()).GetTranslation();
                    cd::Vec4f vec4Position =pSkinMeshComponent->GetBoneChangeMatrix(pSceneWorld->GetSelectedBoneID().Data()) * (cd::Vec4f(position.x(),position.y(),position.z(),1.0f));
                    m_pCameraController->CameraFocus();
                }
            }
        }
    }
    ImGui::SameLine();
    ImGui::Text(reinterpret_cast<const char*>(ICON_MDI_BONE));
    if (isOpen)
    {
        for (auto& child : Bone.GetChildIDs())
        {
            const cd::Bone& bone = pSceneDatabase->GetBone(child.Data());
            DrawBone(pSceneWorld, bone);
        }

        ImGui::TreePop();
    }
}

void SkeletonView::DrawSkeleton(engine::SceneWorld* pSceneWorld)
{
    cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase();
    if (0 == pSceneDatabase->GetBoneCount())
    {
        return;
    }
    const cd::Bone& rootBone = pSceneDatabase->GetBone(0);
    DrawBone(pSceneWorld, rootBone);
}

void SkeletonView::Update()
{
    ImGui::Begin(GetName(), &m_isEnable);
    engine::SceneWorld* pSceneWorld = GetSceneWorld();
    engine::Entity selectedEntity = pSceneWorld->GetSelectedEntity();
    if (engine::INVALID_ENTITY == selectedEntity)
    {
        ImGui::End();
        return;
    }
    engine::AnimationComponent* pAnimationConponent = pSceneWorld->GetAnimationComponent(selectedEntity);
    if (pAnimationConponent)
    {
        DrawSkeleton(pSceneWorld);
    }

    ImGui::End();
}

}