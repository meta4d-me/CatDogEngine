#include "Core/Delegates/MulticastDelegate.hpp"
#include "Display/CameraController.h"
#include "ECWorld/Entity.h"
#include "ImGui/ImGuiBaseLayer.h"

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

class SceneView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~SceneView();

	virtual void Init() override;
	virtual void Update() override;

	engine::MulticastDelegate<void(uint16_t, uint16_t)> OnResize;


	void PickSceneMesh(float regionWidth, float regionHeight);

	ImGuizmo::OPERATION GetImGuizmoOperation() const { return m_currentOperation; }

	void SetCameraController(engine::CameraController* pCameraController) { m_pCameraController = pCameraController; }

private:
	void UpdateToolMenuButtons();
	void Update2DAnd3DButtons();
	void UpdateSwitchAABBButton();
	void UpdateOperationButtons();

private:
	uint16_t m_lastContentWidth = 0;
	uint16_t m_lastContentHeight = 0;

	ImGuizmo::OPERATION m_currentOperation;

	bool m_is3DMode = true;
	bool m_isIBLActive = false;

	engine::RenderTarget* m_pRenderTarget = nullptr;
	bool m_isMouseDownFirstTime = true;

	engine::CameraController* m_pCameraController = nullptr;
};

}