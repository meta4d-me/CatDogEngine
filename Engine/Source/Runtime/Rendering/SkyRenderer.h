#pragma once

#include "Renderer.h"

namespace engine
{

class SkyRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void Render(float deltaTime) override;

private:
	bgfx::UniformHandle m_uniformTexCube;
	bgfx::UniformHandle m_uniformTexCubeIrr;
	bgfx::ProgramHandle m_programSky;

	bgfx::TextureHandle m_lightProbeTex;
	bgfx::TextureHandle m_lightProbeTexIrr;
	float m_lightProbeEV100 = 0.0f;
};

}