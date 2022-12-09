#include "ImGui/ImGuiBaseLayer.h"

namespace editor
{

class ImGuizmoView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~ImGuizmoView();

	virtual void Init() override;
	virtual void Update() override;
};

}