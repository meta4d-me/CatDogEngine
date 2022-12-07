#include "ImGui/ImGuiBaseLayer.h"

#include <inttypes.h>

namespace editor
{

class GameView : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~GameView();

	virtual void Init() override;
	virtual void Update() override;
	void OnResize();
};

}