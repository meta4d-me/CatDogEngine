#include "PostProcessRenderer.h"

#include "GBuffer.h"
#include "SwapChain.h"

#include <bx/math.h>

namespace engine
{

void PostProcessRenderer::Init()
{
	// Uniforms
	s_lightingResult = bgfx::createUniform("s_lightingColor", bgfx::UniformType::Sampler);

	// Loading resources
	std::string resourceRootPath = "S:/CatDogEngine/Projects/SponzaBaseScene/Resources";
	std::string texturePath = resourceRootPath + "/Textures/";
	std::string shaderPath = resourceRootPath + "/Shaders/";

	bgfx::ShaderHandle vsh = Renderer::LoadShader(shaderPath + "vs_fullscreen.bin");
	bgfx::ShaderHandle fsh = Renderer::LoadShader(shaderPath + "fs_PBR_postProcessing.bin");
	m_programPostProcessing = bgfx::createProgram(vsh, fsh, true);
}

PostProcessRenderer::~PostProcessRenderer()
{
	bgfx::destroy(s_lightingResult);
	bgfx::destroy(m_programPostProcessing);
}

void PostProcessRenderer::UpdateView()
{
	bgfx::setViewRect(GetViewID(), 0, 0, m_pGBuffer->GetWidth(), m_pGBuffer->GetHeight());

	float proj[16];
	const bgfx::Caps* caps = bgfx::getCaps();
	bx::mtxOrtho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, caps->homogeneousDepth);
	bgfx::setViewTransform(GetViewID(), NULL, proj);
}

void PostProcessRenderer::Render(float deltaTime)
{
	// Output to swap chain
	bgfx::setViewFrameBuffer(GetViewID(), *GetSwapChain()->GetFrameBuffer());

	// Get input texture from GBuffer
	bgfx::setTexture(0, s_lightingResult, bgfx::getTexture(*m_pGBuffer->GetFrameBuffer()));
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(static_cast<float>(m_pGBuffer->GetWidth()), static_cast<float>(m_pGBuffer->GetHeight()), false);
	bgfx::submit(GetViewID(), m_programPostProcessing);
}

}