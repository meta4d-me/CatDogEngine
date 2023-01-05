#include "ImGui/ImGuiBaseLayer.h"

#include "ECWorld/Entity.h"

#include <imgui/imgui.h>

namespace editor
{

class EditorSceneWorld;

class EntityList : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~EntityList();

	virtual void Init() override;
	virtual void Update() override;

	void AddEntity();
	void DrawEntity(engine::Entity entity);
	void SetSceneWorld(EditorSceneWorld* pWorld) { m_pEditorSceneWorld = pWorld; }

private:
	ImGuiTextFilter m_entityFilter;
	EditorSceneWorld* m_pEditorSceneWorld;
};

}