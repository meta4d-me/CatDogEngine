#pragma once

#include "Renderer.h"

namespace engine::Rendering
{

class UIRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void Render(float deltaTime) override;
};

}