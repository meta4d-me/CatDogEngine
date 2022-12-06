#include "PostProcessRenderer.h"

#include "GBuffer.h"
#include "RenderContext.h"
#include "SwapChain.h"

#include <bx/math.h>

namespace engine
{

void PostProcessRenderer::Init()
{
	m_pRenderContext->CreateUniform("s_lightingColor", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateProgram("GBufferToScreen", "vs_fullscreen.bin", "fs_PBR_postProcessing.bin");
}

PostProcessRenderer::~PostProcessRenderer()
{
}

void PostProcessRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	// Output to swap chain
	bgfx::setViewFrameBuffer(GetViewID(), *GetSwapChain()->GetFrameBuffer());
	bgfx::setViewRect(GetViewID(), 0, 0, m_pGBuffer->GetWidth(), m_pGBuffer->GetHeight());

	float proj[16];
	bx::mtxOrtho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(GetViewID(), nullptr, proj);
}

void PostProcessRenderer::Render(float deltaTime)
{
	// Get input texture from GBuffer
	constexpr StringCrc lightingResultSampler("s_lightingColor");
	bgfx::setTexture(0, m_pRenderContext->GetUniform(lightingResultSampler), bgfx::getTexture(*m_pGBuffer->GetFrameBuffer()));
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(static_cast<float>(m_pGBuffer->GetWidth()), static_cast<float>(m_pGBuffer->GetHeight()), false);

	constexpr StringCrc programName("GBufferToScreen");
	bgfx::submit(GetViewID(), m_pRenderContext->GetProgram(programName));
}

}