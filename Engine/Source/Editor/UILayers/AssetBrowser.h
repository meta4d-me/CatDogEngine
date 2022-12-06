#include "ImGui/ImGuiBaseLayer.h"

namespace editor
{

class AssetBrowser : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~AssetBrowser();

	virtual void Init() override;
	virtual void Update() override;
};

}