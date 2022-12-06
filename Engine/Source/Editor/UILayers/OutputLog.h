#include "ImGui/ImGuiBaseLayer.h"

namespace editor
{

class OutputLog : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~OutputLog();

	virtual void Init() override;
	virtual void Update() override;
};

}