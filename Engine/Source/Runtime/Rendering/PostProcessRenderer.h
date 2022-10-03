#pragma once

#include "Renderer.h"
#include "Uniforms.h"

namespace engine
{

class PostProcessRenderer final : public Renderer
{
public:
	using Renderer::Renderer;
	virtual ~PostProcessRenderer();

	virtual void Init() override;
	virtual void UpdateView() override;
	virtual void Render(float deltaTime) override;

private:
	Uniforms m_uniforms;
	bgfx::UniformHandle s_lightingResult;
	bgfx::ProgramHandle m_programPostProcessing;
};

}