#include "ImGui/ImGuiBaseLayer.h"

namespace engine
{

class Profiler : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~Profiler();

	virtual void Init() override;
	virtual void Update() override;
};

}