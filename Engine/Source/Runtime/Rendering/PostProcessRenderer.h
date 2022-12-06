#pragma once

#include "Renderer.h"

namespace engine
{

class PostProcessRenderer final : public Renderer
{
public:
	using Renderer::Renderer;
	virtual ~PostProcessRenderer();

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;
};

}