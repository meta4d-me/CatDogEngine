#include "ImGui/ImGuiBaseLayer.h"
#include "ImGui/ImGuiUtils.hpp"

namespace editor
{

class Inspector : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~Inspector();

	virtual void Init() override;
	virtual void Update() override;

private:
	engine::Entity m_lastSelectedEntity = engine::INVALID_ENTITY;
};

}