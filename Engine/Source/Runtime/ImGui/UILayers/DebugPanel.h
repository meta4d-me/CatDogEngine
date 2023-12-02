#pragma once

#include "ImGui/ImGuiBaseLayer.h"

namespace engine
{

class DebugPanel : public engine::ImGuiBaseLayer
{
public:
	using ImGuiBaseLayer::ImGuiBaseLayer;
	virtual ~DebugPanel();

	virtual void Init() override;
	virtual void Update() override;

private:
	void ShowProfiler();
};

}