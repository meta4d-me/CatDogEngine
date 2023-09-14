#include <imgui_node_editor.h>
#include <imgui/imgui.h>
#include "ImGui/ImGuiBaseLayer.h"
#include <ImGuizmo/ImGuizmo.h>
#include <map>
#include <string>
#include <vector>

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ImGui/ImGuiContextInstance.h"
namespace ax::NodeEditor
{

struct EditorContext;
struct LinkInfo
{
	LinkId Id;
	PinId  InputId;
	PinId  OutputId;
};
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

	ax::NodeEditor::EditorContext* m_Context = nullptr;    
	bool                 m_FirstFrame = true;  
	ImVector<ax::NodeEditor::LinkInfo>   m_Links;      
	int                  m_NextLinkId = 100;     

	std::vector<ax::NodeEditor::PinId> materialNodePinIds;

	std::map<std::string, cd::MaterialTextureType> textureMap;
	std::vector< ax::NodeEditor::NodeId> textureNodeId;
	std::vector< ax::NodeEditor::PinId> textureNodePinId;
	std::vector< int> textureTypeList;
	bool linkSwitch = true;
};

}