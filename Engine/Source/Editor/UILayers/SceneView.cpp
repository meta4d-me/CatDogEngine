#include "SceneView.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "Log/Log.h"
#include "Material/ShaderSchema.h"
#include "Math/Ray.hpp"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderTarget.h"
#include "Scene/SceneDatabase.h"
#include "Window/Input.h"

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <imguizmo/ImGuizmo.h>

namespace
{

struct ImGuizmoOperationMode
{
	// icon font is 16 bits
	const char8_t* pIconFontName;
	const char* pToolStripName;
	ImGuizmo::OPERATION operation;
	bool createUIVerticalLine;
};

constexpr ImGuizmo::OPERATION SelectOperation = static_cast<ImGuizmo::OPERATION>(0);

// Be careful to control the last ImGuizmoOperationMode object's createUIVerticalLine flag.
// It depends on what the next UI element is.
constexpr ImGuizmoOperationMode OperationModes[] = {
	{ ICON_MDI_CURSOR_DEFAULT, "Select",  SelectOperation, true},
	{ ICON_MDI_ARROW_ALL, "Translate",  ImGuizmo::OPERATION::TRANSLATE, false},
	{ ICON_MDI_ROTATE_ORBIT, "Rotate",  ImGuizmo::OPERATION::ROTATE, false},
	{ ICON_MDI_ARROW_EXPAND_ALL, "Scale",  ImGuizmo::OPERATION::SCALE, false},
	{ ICON_MDI_CROP_ROTATE, "Transform",  ImGuizmo::OPERATION::UNIVERSAL, true},
};

}

namespace editor
{

SceneView::~SceneView()
{

}

void SceneView::Init()
{
	constexpr engine::StringCrc sceneRenderTarget("SceneRenderTarget");
	engine::RenderTarget* pRenderTarget = GetRenderContext()->GetRenderTarget(sceneRenderTarget);
	OnResize.Bind<engine::RenderTarget, &engine::RenderTarget::Resize>(pRenderTarget);

	m_currentOperation = SelectOperation;
}

void SceneView::UpdateOperationButtons()
{
	bool isSelected = false;
	for (const auto& operationMode : OperationModes)
	{
		isSelected = operationMode.operation == m_currentOperation;
		if (isSelected)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));
		}

		ImGui::SameLine();
		if (ImGui::Button(reinterpret_cast<const char*>(operationMode.pIconFontName)))
		{
			m_currentOperation = operationMode.operation;
		}

		if (isSelected)
		{
			ImGui::PopStyleColor();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::TextUnformatted(operationMode.pToolStripName);
			ImGui::EndTooltip();
		}
		ImGui::PopStyleVar();

		if (operationMode.createUIVerticalLine)
		{
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
		}
	}
}

void SceneView::Update2DAnd3DButtons()
{
	bool is3DMode = m_is3DMode;
	if (is3DMode)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));
	}

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_AXIS_ARROW " 3D")))
	{
		m_is3DMode = true;
	}

	if (is3DMode)
	{
		ImGui::PopStyleColor();
	}

	ImGui::SameLine();

	is3DMode = m_is3DMode;
	if (!is3DMode)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));
	}

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_ANGLE_RIGHT " 2D")))
	{
		m_is3DMode = false;
	}

	if (!is3DMode)
	{
		ImGui::PopStyleColor();
	}
}

void SceneView::UpdateSwitchIBLButton()
{
	bool isIBLActive = m_isIBLActive;
	if (isIBLActive)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));
	}

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_CUBE " IBL")))
	{
		m_isIBLActive = !m_isIBLActive;
		
		// TODO
	}

	if (isIBLActive)
	{
		ImGui::PopStyleColor();
	}
}

void SceneView::UpdateSwitchAABBButton()
{
	bool isAABBActive = false;
	if (isAABBActive)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));
	}

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_CUBE " AABB")))
	{
		GetRenderContext();
	}

	if (isAABBActive)
	{
		ImGui::PopStyleColor();
	}
}

void SceneView::UpdateToolMenuButtons()
{
	ImGui::Indent();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	UpdateOperationButtons();

	//if (ImGui::Button(reinterpret_cast<const char*>("Options " ICON_MDI_CHEVRON_DOWN)))
	//{
	//	ImGui::OpenPopup("GizmosPopup");
	//}
	//
	//if (ImGui::BeginPopup("GizmosPopup"))
	//{
	//	ImGui::Checkbox("Option 1", &m_option1);
	//	ImGui::Checkbox("Option 2", &m_option2);
	//
	//	ImGui::EndPopup();
	//}

	//ImGui::SameLine();
	//ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
	ImGui::SameLine();

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_CAMERA " FrameAll")))
	{
		engine::SceneWorld* pSceneWorld = GetSceneWorld();
		if (cd::SceneDatabase* pSceneDatabase = pSceneWorld->GetSceneDatabase())
		{
			pSceneDatabase->UpdateAABB();

			engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(pSceneWorld->GetMainCameraEntity());
			pCameraComponent->FrameAll(pSceneDatabase->GetAABB());
		}
	}

	ImGui::SameLine();

	//Update2DAnd3DButtons();
	//ImGui::SameLine();

	//UpdateSwitchIBLButton();
	//ImGui::SameLine();

	UpdateSwitchAABBButton();

	ImGui::PopStyleColor();
}

void SceneView::PickSceneMesh(float regionWidth, float regionHeight)
{
	if (m_currentOperation != SelectOperation)
	{
		return;
	}

	float screenX = static_cast<float>(engine::Input::Get().GetMousePositionX() - GetWindowPosX());
	float screenY = static_cast<float>(engine::Input::Get().GetMousePositionY() - GetWindowPosY());
	float screenWidth = static_cast<float>(regionWidth);
	float screenHeight = static_cast<float>(regionHeight);
	if (screenX < 0.0f || screenX > screenWidth ||
		screenY < 0.0f || screenY > screenHeight)
	{
		return;
	}

	// Loop through scene's all static meshes' AABB to test intersections with Ray.
	engine::SceneWorld* pSceneWorld = GetSceneWorld();
	engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(pSceneWorld->GetMainCameraEntity());
	cd::Ray pickRay = pCameraComponent->EmitRay(screenX, screenY, screenWidth, screenHeight);

	float minRayTime = FLT_MAX;
	engine::Entity nearestEntity = engine::INVALID_ENTITY;
	for (engine::Entity entity : pSceneWorld->GetStaticMeshEntities())
	{
		engine::TransformComponent* pTransformComponent = pSceneWorld->GetTransformComponent(entity);
		if (!pTransformComponent)
		{
			continue;
		}

		engine::StaticMeshComponent* pMeshComponent = pSceneWorld->GetStaticMeshComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		cd::AABB collisonMeshAABB = pMeshComponent->GetAABB();
		collisonMeshAABB = collisonMeshAABB.Transform(pTransformComponent->GetWorldMatrix());

		float rayTime;
		if (collisonMeshAABB.Intersects(pickRay, rayTime))
		{
			if (rayTime < minRayTime)
			{
				minRayTime = rayTime;
				nearestEntity = entity;
			}
		}
	}

	pSceneWorld->SetSelectedEntity(nearestEntity);
}

void SceneView::Update()
{
	engine::SceneWorld* pSceneWorld = GetSceneWorld();
	engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(pSceneWorld->GetMainCameraEntity());

	if (nullptr == m_pRenderTarget)
	{
		constexpr engine::StringCrc sceneRenderTarget("SceneRenderTarget");
		m_pRenderTarget = GetRenderContext()->GetRenderTarget(sceneRenderTarget);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin(GetName(), &m_isEnable, flags);

	// Draw top menu buttons which include ImGuizmo operation modes, ViewCamera settings.
	UpdateToolMenuButtons();

	// Check if need to resize scene view.
	ImVec2 regionSize = ImGui::GetContentRegionAvail();
	uint16_t regionWidth = static_cast<uint16_t>(regionSize.x);
	uint16_t regionHeight = static_cast<uint16_t>(regionSize.y);
	if (regionWidth != m_lastContentWidth || regionHeight != m_lastContentHeight)
	{
		// RenderTarget binds to OnResize delegate so it will resize automatically.
		// Then RenderTarget will resize ViewCamera/ImGuiContext to let everything go well.
		OnResize.Invoke(regionWidth, regionHeight);
		m_lastContentWidth = regionWidth;
		m_lastContentHeight = regionHeight;
		if (!pCameraComponent->DoConstrainAspectRatio())
		{
			pCameraComponent->SetAspect(static_cast<float>(regionWidth) / static_cast<float>(regionHeight));
		}
	}

	// Check if mouse hover on the area of SceneView so it can control.
	ImVec2 cursorPosition = ImGui::GetCursorPos();
	ImVec2 sceneViewPosition = ImGui::GetWindowPos() + cursorPosition;
	SetWindowPos(sceneViewPosition.x, sceneViewPosition.y);

	// Draw scene.
	ImGui::Image(ImTextureID(m_pRenderTarget->GetTextureHandle(0).idx),
		ImVec2(m_pRenderTarget->GetWidth(), m_pRenderTarget->GetHeight()));

	// Check if there is a file to drop in the scene view to import assets automatically.
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("AssetFile", ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
		{
			std::string filePath(static_cast<const char*>(pPayload->Data));
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::PopStyleVar();

	ImGui::End();

	if (engine::Input::Get().IsMouseLBPressed())
	{
		if (!m_isMouseDownFirstTime)
		{
			return;
		}

		m_isMouseDownFirstTime = false;
		PickSceneMesh(regionWidth, regionHeight);
	}
	else
	{
		m_isMouseDownFirstTime = true;
	}
}

}