#pragma once

#include "ImGui/ImGuiBaseLayer.h"

namespace ax::NodeEditor
{

struct EditorContext;

}

namespace editor
{

class TestNodeEditor : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~TestNodeEditor();

	virtual void Init() override;
	virtual void Update() override;

private:
	ax::NodeEditor::EditorContext* m_pNodeEditorContext = nullptr;
};

}