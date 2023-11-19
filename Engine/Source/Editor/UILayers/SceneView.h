#pragma once

#include "Camera/ViewportCameraController.h"
#include "Core/Delegates/MulticastDelegate.hpp"
#include "ECWorld/Entity.h"
#include "ImGui/ImGuiBaseLayer.h"
#include "Rendering/AABBRenderer.h"
#include "Rendering/WhiteModelRenderer.h"

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <ImGuizmo/ImGuizmo.h>

#include <cstdint>

namespace engine
{

class Renderer;
class RenderTarget;

}

namespace editor
{

enum class RenderModeType
{
	Rendered,
	Solid,
	Wireframe,

	Count,
};

constexpr const char* RenderModes[] = {
	"Rendered",
	"Solid",
	"Wireframe",
};

static_assert(static_cast<int>(RenderModeType::Count) == sizeof(RenderModes) / sizeof(char*),
	"Debug mode type and names mismatch.");

class SceneView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~SceneView();

	virtual void Init() override;
	virtual void Update() override;

	virtual std::pair<float, float> GetWorkRectPosition() const override;
	virtual std::pair<float, float> GetWorkRectSize() const override;

	engine::MulticastDelegate<void(uint16_t, uint16_t)> OnResize;

	ImGuizmo::OPERATION GetImGuizmoOperation() const { return m_currentOperation; }
	RenderModeType GetRenderMode() const { return m_renderMode; }

	void SetSceneRenderer(engine::Renderer* pSceneRenderer) { m_pSceneRenderer = pSceneRenderer; }
	void SetWhiteModelRenderer(engine::Renderer* pWhiteModelRenderer) { m_pWhiteModelRenderer = pWhiteModelRenderer; }
	void SetWireframeRenderer(engine::Renderer* pWireframeRenderer) { m_pWireframeRenderer = pWireframeRenderer; }
	void SetAABBRenderer(engine::Renderer* pAABBRenderer) { m_pAABBRenderer = pAABBRenderer; }
	
	bool IsTerrainEditMode() const { return m_isTerrainEditMode; }

	void SetCameraController(engine::ViewportCameraController* pCameraController) { m_pCameraController = pCameraController; }
	const engine::RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }

	bool OnMouseDown(float x, float y);
	void PickSceneMesh(float x, float y);

private:
	void UpdateToolMenuButtons();
	void Update2DAnd3DButtons();
	void UpdateSwitchAABBButton();
	void UpdateSwitchTerrainButton();
	void UpdateOperationButtons();
	void UpdateRenderModeCombo();

private:
	uint16_t m_lastContentWidth = 0;
	uint16_t m_lastContentHeight = 0;

	ImGuizmo::OPERATION m_currentOperation;

	float m_workRectPosX = 0.0f;
	float m_workRectPosY = 0.0f;
	float m_workRectWidth = 1.0f;
	float m_workRectHeight = 1.0f;

	bool m_is3DMode = true;
	bool m_isIBLActive = false;
	bool m_isTerrainEditMode = false;
	RenderModeType m_renderMode = RenderModeType::Rendered;

	engine::Renderer* m_pSceneRenderer = nullptr;
	engine::Renderer* m_pWhiteModelRenderer = nullptr;
	engine::Renderer* m_pWireframeRenderer = nullptr;
	engine::Renderer* m_pAABBRenderer = nullptr;

	engine::RenderTarget* m_pRenderTarget = nullptr;
	engine::ViewportCameraController* m_pCameraController = nullptr;
};

}