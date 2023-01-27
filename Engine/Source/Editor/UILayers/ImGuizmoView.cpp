#include "ImGuizmoView.h"

#include "Display/Camera.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ImGui/ImGuiContextInstance.h"
#include "Rendering/RenderContext.h"

// TODO : can use StringCrc to access other UILayers from ImGuiContextInstance.
#include "UILayers/SceneView.h"

#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>

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
	ImGuiIO& io = ImGui::GetIO();
	engine::ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
	engine::SceneWorld* pSceneWorld = pImGuiContextInstance->GetSceneWorld();
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
	engine::RenderContext* pRenderContext = reinterpret_cast<engine::RenderContext*>(io.BackendRendererUserData);
	engine::Camera* pCamera = pRenderContext->GetCamera();

	ImGuizmo::BeginFrame();
	constexpr bool isPerspective = true;
	ImGuizmo::SetOrthographic(!isPerspective);
	ImGuizmo::SetRect(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y);
	cd::Matrix4x4 worldMatrix = pTransformComponent->GetWorldMatrix();
	ImGuizmo::Manipulate(pCamera->GetViewMatrix().Begin(), pCamera->GetProjectionMatrix().Begin(),
		operation, ImGuizmo::LOCAL, worldMatrix.Begin());

	if (ImGuizmo::IsUsing())
	{
		if (ImGuizmo::OPERATION::TRANSLATE & operation)
		{
			pTransformComponent->GetTransform().SetTranslation(worldMatrix.GetTranslation());
		}
		
		if (ImGuizmo::OPERATION::ROTATE & operation)
		{
			cd::Quaternion rotation = cd::Quaternion::FromMatrix(worldMatrix.GetRotation());
			pTransformComponent->GetTransform().SetRotation(cd::MoveTemp(rotation));
		}

		if (ImGuizmo::OPERATION::SCALE & operation)
		{
			pTransformComponent->GetTransform().SetScale(worldMatrix.GetScale());
		}

		pTransformComponent->Build();
	}
}

}