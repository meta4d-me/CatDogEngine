#include "ImGuizmoView.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkinMeshComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ImGui/ImGuiContextInstance.h"

// TODO : can use StringCrc to access other UILayers from ImGuiContextInstance.
#include "UILayers/SceneView.h"

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>

namespace editor
{

ImGuizmoView::~ImGuizmoView()
{
}

void ImGuizmoView::Init()
{
	ImGuizmo::SetGizmoSizeClipSpace(0.25f);
}

void ImGuizmoView::Update()
{
	engine::SceneWorld* pSceneWorld = GetSceneWorld();
	cd::SceneDatabase* pSceneDatabese = pSceneWorld->GetSceneDatabase();
	engine::Entity selectedEntity = pSceneWorld->GetSelectedEntity();
	cd::BoneID selectedBoneID = pSceneWorld->GetSelectedBoneID();

	if (engine::INVALID_ENTITY == selectedEntity && cd::BoneID::InvalidID == selectedBoneID)
	{
		return;
	}

	cd::Matrix4x4 deltaMatrix = cd::Matrix4x4::Identity();
	cd::Matrix4x4 worldMatrix = cd::Matrix4x4::Identity();
	engine::TransformComponent* pTransformComponent = pSceneWorld->GetTransformComponent(selectedEntity);
	engine::SkinMeshComponent* pSkinMeshComponent = pSceneWorld->GetSkinMeshComponent(selectedEntity);

	ImGuizmo::OPERATION operation = m_pSceneView->GetImGuizmoOperation();
	const engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(pSceneWorld->GetMainCameraEntity());

	ImGuizmo::BeginFrame();
	constexpr bool isPerspective = true;
	ImGuizmo::SetOrthographic(!isPerspective);
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0.0f, 0.0f, ImGui::GetIO().DisplaySize.x, io.DisplaySize.y);
	if (engine::INVALID_ENTITY != selectedEntity && pTransformComponent)
	{
		cd::Matrix4x4 worldMatrix = pTransformComponent->GetWorldMatrix();
		ImGuizmo::Manipulate(pCameraComponent->GetViewMatrix().Begin(), pCameraComponent->GetProjectionMatrix().Begin(),
			operation, ImGuizmo::LOCAL, worldMatrix.Begin());
	}
	 if (cd::BoneID::InvalidID != selectedBoneID && pSkinMeshComponent)
	{
		uint32_t index = selectedBoneID.Data();
		cd::Matrix4x4 worldMatrix = pSkinMeshComponent->GetBoneMatrix(index);
		ImGuizmo::Manipulate(pCameraComponent->GetViewMatrix().Begin(), pCameraComponent->GetProjectionMatrix().Begin(),
			operation, ImGuizmo::LOCAL, worldMatrix.Begin(), deltaMatrix.Begin());
	}

	if (ImGuizmo::IsUsing())
	{
		if (ImGuizmo::OPERATION::TRANSLATE & operation)
		{
			if (pTransformComponent)
			{
				pTransformComponent->GetTransform().SetTranslation(worldMatrix.GetTranslation());
				pTransformComponent->Dirty();
			}

			if (pSkinMeshComponent)
			{
				pSkinMeshComponent->SetBoneChangeMatrix(selectedBoneID.Data(), deltaMatrix);
				pSkinMeshComponent->SetChangeBoneIndex(selectedBoneID.Data());
			}
			
		}

		if (ImGuizmo::OPERATION::ROTATE & operation)
		{
			pTransformComponent->GetTransform().SetRotation(cd::Quaternion::FromMatrix(worldMatrix.GetRotation()));
			pTransformComponent->Dirty();
			if (pSkinMeshComponent)
			{
				pSkinMeshComponent->SetBoneChangeMatrix(selectedBoneID.Data(), deltaMatrix);
				pSkinMeshComponent->SetBoneMatrix(selectedBoneID.Data(), worldMatrix);
				pSkinMeshComponent->SetChangeBoneIndex(selectedBoneID.Data());
			}
		}

		if (ImGuizmo::OPERATION::SCALE & operation)
		{
			pTransformComponent->GetTransform().SetScale(worldMatrix.GetScale());
			pTransformComponent->Dirty();
		}

		pTransformComponent->Build();
	}
}

}