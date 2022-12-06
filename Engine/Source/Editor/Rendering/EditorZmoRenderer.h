#pragma once

#include "Rendering/Renderer.h"

namespace editor
{

class EditorZmoRenderer final : public engine::Renderer
{
public:
	using Renderer::Renderer;
	virtual ~EditorZmoRenderer();

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;
};

}