#include "Core/Delegates/MulticastDelegate.hpp"
#include "ImGui/ImGuiBaseLayer.h"

#include <inttypes.h>

namespace ImGuizmo
{

enum OPERATION;

}

namespace engine
{

class RenderTarget;

}

namespace editor
{

class EditorSceneWorld;

class SceneView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~SceneView();

	virtual void Init() override;
	virtual void Update() override;

	engine::MulticastDelegate<void(uint16_t, uint16_t)> OnResize;
	void SetSceneWorld(EditorSceneWorld* pWorld) { m_pEditorSceneWorld = pWorld; }

private:
	void UpdateToolMenuButtons();
	
private:
	EditorSceneWorld* m_pEditorSceneWorld;

	uint16_t m_lastContentWidth = 0;
	uint16_t m_lastContentHeight = 0;

	bool m_option1;
	bool m_option2;
	ImGuizmo::OPERATION m_currentOperation;

	engine::RenderTarget* m_pRenderTarget = nullptr;
};

}