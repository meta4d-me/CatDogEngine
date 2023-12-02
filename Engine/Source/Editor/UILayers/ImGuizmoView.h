#pragma once

#include "ImGui/ImGuiBaseLayer.h"

namespace editor
{

class SceneView;

class ImGuizmoView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~ImGuizmoView();

	virtual void Init() override;
	virtual void Update() override;

private:
	uint32_t m_sceneViewID;
};

}