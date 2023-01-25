#include "ImGui/ImGuiBaseLayer.h"

#include "ECWorld/Entity.h"

#include <imgui/imgui.h>

namespace editor
{

class EntityList : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~EntityList();

	virtual void Init() override;
	virtual void Update() override;

	void AddEntity();
	void DrawEntity(engine::Entity entity);

private:
	ImGuiTextFilter m_entityFilter;
};

}