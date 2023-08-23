#include "ImGui/ImGuiBaseLayer.h"

#include <cstdint>

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