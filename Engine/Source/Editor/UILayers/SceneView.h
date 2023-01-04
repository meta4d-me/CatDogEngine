#include "Core/Delegates/MulticastDelegate.hpp"
#include "ImGui/ImGuiBaseLayer.h"

#include <inttypes.h>

namespace ImGuizmo
{

enum OPERATION;

}

namespace cd
{

class SceneDatabase;

}

namespace engine
{

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

	void SetSceneDatabase(cd::SceneDatabase* pSceneDatabase) { m_pSceneDatabase = pSceneDatabase; }

public:
	engine::MulticastDelegate<void(uint16_t, uint16_t)> OnResize;

private:
	void UpdateToolMenuButtons();

private:
	cd::SceneDatabase* m_pSceneDatabase = nullptr;

	uint16_t m_lastContentWidth = 0;
	uint16_t m_lastContentHeight = 0;

	bool m_option1;
	bool m_option2;
	ImGuizmo::OPERATION m_currentOperation;

	engine::RenderTarget* m_pRenderTarget = nullptr;
};

}