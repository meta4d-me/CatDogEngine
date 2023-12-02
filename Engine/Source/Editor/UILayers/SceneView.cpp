#include "SceneView.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ImGui/IconFont/IconsMaterialDesignIcons.h"
#include "ImGui/ImGuiContextInstance.h"
#include "Log/Log.h"
#include "Material/ShaderSchema.h"
#include "Math/Ray.hpp"
#include "Rendering/AABBRenderer.h"
#include "Rendering/WireframeRenderer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderTarget.h"
#include "Scene/SceneDatabase.h"
#include "Window/Input.h"

namespace
{

struct ImGuizmoOperationMode
{
#ifdef __clang__
	const char* pIconFontName;
#else
	const char8_t* pIconFontName;
#endif // __clang__

	// icon font is 16 bits
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

void SceneView::UpdateRenderModeCombo()
{
	ImGui::SetNextItemWidth(130);
	if (ImGui::Combo("##DebugCombo", reinterpret_cast<int*>(&m_renderMode), RenderModes, IM_ARRAYSIZE(RenderModes)))
	{
		m_pSceneRenderer->SetEnable(RenderModeType::Rendered == m_renderMode);
		m_pWhiteModelRenderer->SetEnable(RenderModeType::Solid == m_renderMode);

		auto* pWireframeRenderer = static_cast<engine::WireframeRenderer*>(m_pWireframeRenderer);
		pWireframeRenderer->SetEnableGlobalWireframe(RenderModeType::Wireframe == m_renderMode);
	}
	ImGui::PushItemWidth(130);
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

void SceneView::UpdateSwitchAABBButton()
{
	bool isAABBActive = false;
	if (isAABBActive)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));
	}

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_CUBE " AABB")))
	{
		auto* pAABBRenderer = static_cast<engine::AABBRenderer*>(m_pAABBRenderer);
		pAABBRenderer->SetEnableGlobalAABB(!pAABBRenderer->IsGlobalAABBEnable());
	}

	if (isAABBActive)
	{
		ImGui::PopStyleColor();
	}
}

void SceneView::UpdateSwitchTerrainButton()
{
	bool isTerrainActive = m_isTerrainEditMode;
	if (isTerrainActive)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));
	}

	if (ImGui::Button(reinterpret_cast<const char*>(ICON_MDI_CUBE "Smooth Terrain ")))
	{
		m_isTerrainEditMode = !m_isTerrainEditMode;
	}

	if (isTerrainActive)
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
	
	//Update2DAnd3DButtons();
	//ImGui::SameLine();

	//UpdateSwitchIBLButton();
	//ImGui::SameLine();

	ImGui::SameLine();
	UpdateRenderModeCombo();

	ImGui::SameLine();
	UpdateSwitchAABBButton();

	ImGui::SameLine();
	UpdateSwitchTerrainButton();

	ImGui::PopStyleColor();
}

void SceneView::PickSceneMesh(float x, float y)
{
	// Loop through scene's all static meshes' AABB to test intersections with Ray.
	engine::SceneWorld* pSceneWorld = GetSceneWorld();
	engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(pSceneWorld->GetMainCameraEntity());
	ImVec2 mousePos = ImGui::GetMousePos();
	cd::Ray pickRay = pCameraComponent->EmitRay(mousePos.x - m_workRectPosX, mousePos.y - m_workRectPosY, m_workRectWidth, m_workRectHeight);

	float minRayTime = FLT_MAX;
	engine::Entity nearestEntity = engine::INVALID_ENTITY;
	for (engine::Entity entity : pSceneWorld->GetCollisionMeshEntities())
	{
		engine::TransformComponent* pTransformComponent = pSceneWorld->GetTransformComponent(entity);
		if (!pTransformComponent)
		{
			continue;
		}

		auto* pCollisionMesh = pSceneWorld->GetCollisionMeshComponent(entity);
		if (!pCollisionMesh)
		{
			continue;
		}

		cd::AABB collisonMeshAABB = pCollisionMesh->GetAABB();
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

bool SceneView::OnMouseDown(float x, float y)
{
	// Operations
	if (ImGuizmo::IsUsing())
	{
	}
	else
	{
		// Pick
		if (m_currentOperation == SelectOperation && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			PickSceneMesh(x, y);
			return true;
		}
	
		// Focus
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) &&
			engine::INVALID_ENTITY != GetSceneWorld()->GetSelectedEntity())
		{
			m_pCameraController->CameraFocus();
		}
	}

	return false;
}

void SceneView::Update()
{
	engine::SceneWorld* pSceneWorld = GetSceneWorld();
	engine::CameraComponent* pCameraComponent = pSceneWorld->GetCameraComponent(pSceneWorld->GetMainCameraEntity());
	engine::TerrainComponent* pTerrainComponent = pSceneWorld->GetTerrainComponent(pSceneWorld->GetSelectedEntity());

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
	ImVec2 currentPos = GetRootWindow()->DC.CursorPos;
	ImVec2 regionSize = ImGui::GetContentRegionAvail();
	m_workRectPosX = currentPos.x;
	m_workRectPosY = currentPos.y;
	m_workRectWidth = regionSize.x;
	m_workRectHeight = regionSize.y;

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
			pCameraComponent->BuildProjectMatrix();
		}
	}

	// Draw scene : index 0 should be SceneColor.
	ImGui::Image(reinterpret_cast<ImTextureID>(m_pRenderTarget->GetTextureHandle(0).idx), ImVec2(m_pRenderTarget->GetWidth(), m_pRenderTarget->GetHeight()));

	ImGui::PopStyleVar();
	ImGui::End();
}

std::pair<float, float> SceneView::GetWorkRectPosition() const
{
	return std::make_pair(m_workRectPosX, m_workRectPosY);
}

std::pair<float, float> SceneView::GetWorkRectSize() const
{
	return std::make_pair(m_workRectWidth, m_workRectHeight);
}

}