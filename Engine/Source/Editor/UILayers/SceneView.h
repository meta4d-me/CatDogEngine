#include "Core/Delegates/MulticastDelegate.hpp"
#include "Display/CameraController.h"
#include "ECWorld/Entity.h"
#include "ImGui/ImGuiBaseLayer.h"
#include "Rendering/AABBRenderer.h"
#include "Rendering/CelluloidRenderer.h"
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

	engine::MulticastDelegate<void(uint16_t, uint16_t)> OnResize;

	void PickSceneMesh(float regionWidth, float regionHeight);

	ImGuizmo::OPERATION GetImGuizmoOperation() const { return m_currentOperation; }
	RenderModeType GetRenderMode() const { return m_renderMode; }

	void SetSceneRenderer(engine::Renderer* pSceneRenderer) { m_pSceneRenderer = pSceneRenderer; }
	void SetWhiteModelRenderer(engine::Renderer* pWhiteModelRenderer) { m_pWhiteModelRenderer = pWhiteModelRenderer; }
	void SetWireframeRenderer(engine::Renderer* pWireframeRenderer) { m_pWireframeRenderer = pWireframeRenderer; }
	void SetAABBRenderer(engine::Renderer* pAABBRenderer) { m_pAABBRenderer = pAABBRenderer; }
	
	bool IsTerrainEditMode() const { return m_isTerrainEditMode; }

	bool IsFirstClick() const { return m_isMouseDownFirstTime; }
	bool IsShowMouse() { return m_isMouseShow; }
	uint32_t GetMouseFixedPositionX() const { return m_mouseFixedPositionX; }
	uint32_t GetMouseFixedPositionY() const { return m_mouseFixedPositionY; }

	void SetCameraController(engine::CameraController* pCameraController) { m_pCameraController = pCameraController; }

	const engine::RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }
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

	bool m_is3DMode = true;
	bool m_isIBLActive = false;
	bool m_isTerrainEditMode = false;
	bool m_isMouseShow = true;
	bool m_isUsingCamera = false;
	bool m_isLeftClick = false;
	RenderModeType m_renderMode = RenderModeType::Rendered;

	engine::Renderer* m_pSceneRenderer = nullptr;
	engine::Renderer* m_pWhiteModelRenderer = nullptr;
	engine::Renderer* m_pWireframeRenderer = nullptr;
	engine::Renderer* m_pAABBRenderer = nullptr;

	engine::RenderTarget* m_pRenderTarget = nullptr;
	bool m_isMouseDownFirstTime = true;

	int32_t m_mouseFixedPositionX = 0;
	int32_t m_mouseFixedPositionY = 0;

	engine::CameraController* m_pCameraController = nullptr;
};

}