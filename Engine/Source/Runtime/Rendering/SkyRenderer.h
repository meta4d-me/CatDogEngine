#pragma once

#include "Renderer.h"
#include "Uniforms.h"

namespace engine
{

class SkyRenderer final : public Renderer
{
public:
	using Renderer::Renderer;
	virtual ~SkyRenderer();

	virtual void Init() override;
	virtual void UpdateView() override;
	virtual void Render(float deltaTime) override;

	void RenderForOtherView() const;

private:
	Uniforms m_uniforms;

	bgfx::UniformHandle m_uniformTexLUT;
	bgfx::UniformHandle m_uniformTexCube;
	bgfx::UniformHandle m_uniformTexCubeIrr;
	bgfx::ProgramHandle m_programSky;

	bgfx::TextureHandle m_iblLUTTex;
	bgfx::TextureHandle m_lightProbeTex;
	bgfx::TextureHandle m_lightProbeTexIrr;
	float m_lightProbeEV100 = 0.0f;
};

}