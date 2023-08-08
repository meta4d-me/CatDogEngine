#include "ImGuizmoView.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
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
	std::vector<engine::Entity> selectedEntities = pSceneWorld->GetSelectedEntities();
	if (selectedEntities.empty())
	{
		return;
	}
	ImGuizmo::OPERATION operation = m_pSceneView->GetImGuizmoOperation();
	const engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(pSceneWorld->GetMainCameraEntity());

	ImGuizmo::BeginFrame();
	constexpr bool isPerspective = true;
	ImGuizmo::SetOrthographic(!isPerspective);
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0.0f, 0.0f, ImGui::GetIO().DisplaySize.x, io.DisplaySize.y);
	cd::Matrix4x4 worldMatrix = pSceneWorld->GetTransformComponent(selectedEntities[0])->GetWorldMatrix();
	cd::Matrix4x4 deltaMatrix(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
	ImGuizmo::Manipulate(pCameraComponent->GetViewMatrix().Begin(), pCameraComponent->GetProjectionMatrix().Begin(),
		operation, ImGuizmo::LOCAL, worldMatrix.Begin(), deltaMatrix.Begin());
	for (const auto& entity : selectedEntities)
	{
		engine::TransformComponent* pTransformComponent = pSceneWorld->GetTransformComponent(entity);
		if (!pTransformComponent)
		{
			return;
		}

		if (ImGuizmo::IsUsing())
		{
			if (ImGuizmo::OPERATION::TRANSLATE & operation)
			{
				cd::Vec3f translation = pSceneWorld->GetTransformComponent(entity)->GetTransform().GetTranslation();
				pTransformComponent->GetTransform().SetTranslation(translation + deltaMatrix.GetTranslation());
				pTransformComponent->Dirty();
			}

			if (ImGuizmo::OPERATION::ROTATE & operation)
			{
				cd::Quaternion rotation = pSceneWorld->GetTransformComponent(entity)->GetTransform().GetRotation();
				pTransformComponent->GetTransform().SetRotation(rotation * cd::Quaternion::FromMatrix(deltaMatrix.GetRotation()));
				pTransformComponent->Dirty();
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

}