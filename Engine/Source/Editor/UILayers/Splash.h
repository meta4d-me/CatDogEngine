#include "ImGui/ImGuiBaseLayer.h"

#include <cstdint>

namespace editor
{

class Splash : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~Splash();

	virtual void Init() override;
	virtual void Update() override;

private:
	size_t m_shaderBuildTaskTotalCount = 0;
};

}