#include "ImGui/ImGuiBaseLayer.h"
#include "UILayers/ImGuiUtils.hpp"
namespace editor
{

class Inspector : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~Inspector();

	virtual void Init() override;
	virtual void Update() override;
};

}