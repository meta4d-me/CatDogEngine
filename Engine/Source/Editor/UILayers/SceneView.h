#include "Core/Delegates/MulticastDelegate.hpp"
#include "ECWorld/Entity.h"
#include "ImGui/ImGuiBaseLayer.h"

#include <inttypes.h>

namespace ImGuizmo
{

enum OPERATION;

}

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
	void SetAABBRenderer(engine::Renderer* pRenderer) { m_pAABBRenderer = pRenderer; }
	void SetPBRSkyRenderer(engine::Renderer* pRenderer) { m_pPBRSkyRenderer = pRenderer; }
	void SetIBLSkyRenderer(engine::Renderer* pRenderer) { m_pIBLSkyRenderer = pRenderer; }

	void PickSceneMesh(float regionWidth, float regionHeight);

	ImGuizmo::OPERATION GetImGuizmoOperation() const { return m_currentOperation; }

private:
	void UpdateToolMenuButtons();
	void Update2DAnd3DButtons();
	void UpdateSwitchIBLButton();
	void UpdateSwitchAABBButton();
	void UpdateOperationButtons();
	
private:
	uint16_t m_lastContentWidth = 0;
	uint16_t m_lastContentHeight = 0;

	bool m_option1;
	bool m_option2;
	ImGuizmo::OPERATION m_currentOperation;

	bool m_is3DMode = true;
	bool m_isIBLActive = false;

	engine::Renderer* m_pAABBRenderer = nullptr;
	engine::Renderer* m_pPBRSkyRenderer = nullptr;
	engine::Renderer* m_pIBLSkyRenderer = nullptr;
	engine::RenderTarget* m_pRenderTarget = nullptr;
	bool m_isMouseDownFirstTime = true;

	engine::Entity m_selectedEntity = engine::INVALID_ENTITY;
};

}