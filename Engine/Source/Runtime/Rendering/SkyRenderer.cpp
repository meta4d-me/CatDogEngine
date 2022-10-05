#include "SkyRenderer.h"

#include "GBuffer.h"
#include "SwapChain.h"

#include <bx/math.h>

namespace engine
{

void SkyRenderer::Init()
{
	m_uniforms.Init();
	m_uniformTexCube = bgfx::createUniform("s_texCube", bgfx::UniformType::Sampler);
	m_uniformTexCubeIrr = bgfx::createUniform("s_texCubeIrr", bgfx::UniformType::Sampler);

	// Loading resources
	std::string resourceRootPath = "S:/CatDogEngine/Projects/SponzaBaseScene/Resources";
	std::string texturePath = resourceRootPath + "/Textures/";
	std::string shaderPath = resourceRootPath + "/Shaders/";

	bgfx::ShaderHandle vsh = Renderer::LoadShader(shaderPath + "vs_PBR_skybox.bin");
	bgfx::ShaderHandle fsh = Renderer::LoadShader(shaderPath + "fs_PBR_skybox.bin");
	m_programSky = bgfx::createProgram(vsh, fsh, true);

	uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
	m_lightProbeTex = Renderer::LoadTexture(texturePath + "bolonga_lod.dds", samplerFlags);
	m_lightProbeTexIrr = Renderer::LoadTexture(texturePath + "bolonga_irr.dds", samplerFlags);
	m_lightProbeEV100 = -2.0f;
}

SkyRenderer::~SkyRenderer()
{
	bgfx::destroy(m_uniformTexCube);
	bgfx::destroy(m_uniformTexCubeIrr);
	bgfx::destroy(m_programSky);
	bgfx::destroy(m_lightProbeTex);
	bgfx::destroy(m_lightProbeTexIrr);
}

void SkyRenderer::UpdateView()
{
	bgfx::setViewFrameBuffer(GetViewID(), *m_pGBuffer->GetFrameBuffer());
	bgfx::setViewRect(GetViewID(), 0, 0, m_pGBuffer->GetWidth(), m_pGBuffer->GetHeight());
	
	float proj[16];
	const bgfx::Caps* caps = bgfx::getCaps();
	bx::mtxOrtho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0, caps->homogeneousDepth);
	bgfx::setViewTransform(GetViewID(), NULL, proj);
}

void SkyRenderer::Render(float deltaTime)
{
	m_uniforms.Submit();

	bgfx::setTexture(0, m_uniformTexCube, m_lightProbeTex);
	bgfx::setTexture(1, m_uniformTexCubeIrr, m_lightProbeTexIrr);
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(static_cast<float>(m_pGBuffer->GetWidth()), static_cast<float>(m_pGBuffer->GetHeight()), true);
	bgfx::submit(GetViewID(), m_programSky);
}

}