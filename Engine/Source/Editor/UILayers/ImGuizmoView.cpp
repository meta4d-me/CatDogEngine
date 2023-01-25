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
	ImGuizmo::SetGizmoSizeClipSpace(0.5f);
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

	ImGuizmo::BeginFrame();

	constexpr bool isPerspective = true;
	ImGuizmo::SetOrthographic(!isPerspective);
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	engine::TransformComponent* pTransformComponent = pSceneWorld->GetTransformComponent(selectedEntity);
	if (!pTransformComponent)
	{
		return;
	}

	engine::StaticMeshComponent* pStaticMeshComponent = pSceneWorld->GetStaticMeshComponent(selectedEntity);
	if (!pStaticMeshComponent)
	{
		return;
	}

	cd::Transform deltaTransform = cd::Transform::Identity();
	deltaTransform.SetTranslation(pStaticMeshComponent->GetAABB().Center());

	ImGuizmo::OPERATION operation = m_pSceneView->GetImGuizmoOperation();
	cd::Matrix4x4 worldMatrix = pTransformComponent->GetWorldMatrix();
	engine::RenderContext* pRenderContext = reinterpret_cast<engine::RenderContext*>(io.BackendRendererUserData);
	engine::Camera* pCamera = pRenderContext->GetCamera();
	ImGuizmo::Manipulate(pCamera->GetViewMatrix().Begin(), pCamera->GetProjectionMatrix().Begin(),
		operation, ImGuizmo::LOCAL, worldMatrix.Begin(), deltaTransform.GetMatrix().Begin());

	if (ImGuizmo::IsUsing())
	{
		if (ImGuizmo::OPERATION::SCALE == operation)
		{
			pTransformComponent->GetTransform().SetScale(worldMatrix.GetScale());
		}
		else if (ImGuizmo::OPERATION::TRANSLATE == operation)
		{
			pTransformComponent->GetTransform().SetTranslation(worldMatrix.GetTranslation());
		}
		else if (ImGuizmo::OPERATION::ROTATE == operation)
		{
			pTransformComponent->GetTransform().SetRotation(cd::Quaternion(worldMatrix.GetRotation()));
		}
		else if (ImGuizmo::OPERATION::UNIVERSAL == operation)
		{
			pTransformComponent->GetTransform().SetScale(worldMatrix.GetScale());
			pTransformComponent->GetTransform().SetTranslation(worldMatrix.GetTranslation());
			pTransformComponent->GetTransform().SetRotation(cd::Quaternion(worldMatrix.GetRotation()));
		}
	}
}

}