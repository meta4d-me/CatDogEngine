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
	engine::Entity selectedEntity = pSceneWorld->GetSelectedEntity();

	if (engine::INVALID_ENTITY == selectedEntity)
	{
		return;
	}

	engine::TransformComponent* pTransformComponent = pSceneWorld->GetTransformComponent(selectedEntity);
	if (!pTransformComponent)
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
	cd::Matrix4x4 worldMatrix = pTransformComponent->GetWorldMatrix();
	ImGuizmo::Manipulate(pCameraComponent->GetViewMatrix().begin(), pCameraComponent->GetProjectionMatrix().begin(),
		operation, ImGuizmo::LOCAL, worldMatrix.begin());

	if (ImGuizmo::IsUsing())
	{
		if (ImGuizmo::OPERATION::TRANSLATE & operation)
		{
			pTransformComponent->GetTransform().SetTranslation(worldMatrix.GetTranslation());
			pTransformComponent->Dirty();
		}
		
		if (ImGuizmo::OPERATION::ROTATE & operation)
		{
			pTransformComponent->GetTransform().SetRotation(cd::Quaternion::FromMatrix(worldMatrix.GetRotation()));
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