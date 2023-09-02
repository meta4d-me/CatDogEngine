#include "TestNodeEditor.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ImGui/ImGuiContextInstance.h"

// TODO : can use StringCrc to access other UILayers from ImGuiContextInstance.
#include "UILayers/SceneView.h"

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace editor
{

TestNodeEditor::~TestNodeEditor()
{
}

void TestNodeEditor::Init()
{
	ImGuizmo::SetGizmoSizeClipSpace(0.25f);

	ed::Config config;
	config.SettingsFile = "Simple.json";
	m_pNodeEditorContext = ed::CreateEditor(&config);
}

void TestNodeEditor::Update()
{
	{
		auto& io = ImGui::GetIO();

		ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

		ImGui::Separator();

		ed::SetCurrentEditor(m_pNodeEditorContext);
		ed::Begin("My Editor", ImVec2(0.0, 0.0f));
		int uniqueId = 1;
		// Start drawing nodes.
		ed::BeginNode(uniqueId++);
		ImGui::Text("Node A");
		ed::BeginPin(uniqueId++, ed::PinKind::Input);
		ImGui::Text("-> In");
		ed::EndPin();
		ImGui::SameLine();
		ed::BeginPin(uniqueId++, ed::PinKind::Output);
		ImGui::Text("Out ->");
		ed::EndPin();
		ed::EndNode();
		ed::End();
		ed::SetCurrentEditor(nullptr);
	}
}

}