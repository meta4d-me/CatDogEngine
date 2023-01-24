#include "ImGuizmoView.h"

#include "Display/Camera.h"
#include "ECWorld/SceneWorld.h"
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

}

void ImGuizmoView::Update()
{
	ImGuiIO& io = ImGui::GetIO();
	engine::RenderContext* pCurrentRenderContext = reinterpret_cast<engine::RenderContext*>(io.BackendRendererUserData);
	engine::Camera* pCamera = pCurrentRenderContext->GetCamera();

	bool isPerspective = true;
	ImGuizmo::SetOrthographic(!isPerspective);
	ImGuizmo::BeginFrame();

	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	// Default grid style is ugly..
	//static const float identityMatrix[16] = {
	//	1.f, 0.f, 0.f, 0.f,
	//	0.f, 1.f, 0.f, 0.f,
	//	0.f, 0.f, 1.f, 0.f,
	//	0.f, 0.f, 0.f, 1.f };
	//ImGuizmo::DrawGrid(pCamera->GetViewMatrix().Begin(), pCamera->GetProjectionMatrix().Begin(), identityMatrix, 100.f);

	engine::ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
	engine::SceneWorld* pSceneWorld = pImGuiContextInstance->GetSceneWorld();
	engine::TransformComponent* pTransformComponent = pSceneWorld->GetTransformComponent(pSceneWorld->GetSelectedEntity());
	if (pTransformComponent)
	{
		ImGuizmo::OPERATION operation = m_pSceneView->GetImGuizmoOperation();
		cd::Matrix4x4 worldMatrix = pTransformComponent->GetWorldMatrix();
		ImGuizmo::Manipulate(pCamera->GetViewMatrix().Begin(), pCamera->GetProjectionMatrix().Begin(),
			operation, ImGuizmo::LOCAL, worldMatrix.Begin());

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
		}
	}
}

}