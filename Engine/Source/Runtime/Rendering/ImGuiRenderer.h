#pragma once

#include "Rendering/Renderer.h"

struct ImDrawData;

namespace engine
{

class ImGuiRenderer final : public engine::Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void Warmup() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

private:
	void RenderDrawData(ImDrawData* pImGuiDrawData, float deltaTime);
};

}