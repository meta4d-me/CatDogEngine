#include "ImGui/ImGuiBaseLayer.h"

namespace editor
{

class EntityList : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~EntityList();

	virtual void Init() override;
	virtual void Update() override;
};

}