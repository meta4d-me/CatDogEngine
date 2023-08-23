#include "Core/Delegates/MulticastDelegate.hpp"
#include "Display/CameraController.h"
#include "ECWorld/Entity.h"
#include "ImGui/ImGuiBaseLayer.h"
#include "Rendering/AABBRenderer.h"
#include "Rendering/DebugRenderer.h"

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

enum class DebugModeType
{
	NoDebug,
	WhiteModel,

	Count,
};

constexpr const char* DebugModes[] = {
	"NoDebug",
	"WhiteModel",
	//"WireFrame" 
};

static_assert(static_cast<int>(DebugModeType::Count) == sizeof(DebugModes) / sizeof(char*),
	"Debug mode type and names mismatch.");

enum class AABBModeType
{
	NoAABB,
	AABBSelected,
	AABBAll,

	Count,
};

constexpr const char* AABBModes[] = {
	"NoAABB",
	"AABBSelected",
	"AABBAll"
};

static_assert(static_cast<int>(AABBModeType::Count) == sizeof(AABBModes) / sizeof(char*),
	"AABB mode type and names mismatch.");

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
	DebugModeType GetDebugMode() const { return m_debugMode; }
	AABBModeType GetAABBMode() const { return m_AABBMode; }

	void SetSceneRenderer(engine::Renderer* pSceneRenderer) { m_pSceneRenderer = pSceneRenderer; }
	void SetDebugRenderer(engine::Renderer* pDebugRenderer) { m_pDebugRenderer = pDebugRenderer; }
	void SetAABBRenderer(engine::Renderer* pAABBRenderer) { m_pAABBRenderer = pAABBRenderer; }
	
	bool IsTerrainEditMode() const { return m_isTerrainEditMode; }

	void SetCameraController(engine::CameraController* pCameraController) { m_pCameraController = pCameraController; }

	const engine::RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }

	float m_MainCameraSpeed  = 160.0f;
private:
	void UpdateToolMenuButtons();
	void Update2DAnd3DButtons();
	void UpdateSwitchAABBButton();
	void UpdateSwitchTerrainButton();
	void UpdateOperationButtons();

	void UpdateDebugCombo();
	void UpdateAABBCombo();

private:
	uint16_t m_lastContentWidth = 0;
	uint16_t m_lastContentHeight = 0;

	ImGuizmo::OPERATION m_currentOperation;

	bool m_is3DMode = true;
	bool m_isIBLActive = false;
	bool m_isTerrainEditMode = false;
	DebugModeType m_debugMode = DebugModeType::NoDebug;
	AABBModeType m_AABBMode = AABBModeType::NoAABB;

	engine::Renderer* m_pSceneRenderer = nullptr;
	engine::Renderer* m_pDebugRenderer = nullptr;
	engine::Renderer* m_pAABBRenderer = nullptr;

	engine::RenderTarget* m_pRenderTarget = nullptr;
	bool m_isMouseDownFirstTime = true;

	float sliderRange[2] = { 0.0f, 1000.0f };
	engine::CameraController* m_pCameraController = nullptr;

};

}