#include "SkyRenderer.h"

#include "RenderContext.h"

namespace engine
{

void SkyRenderer::Init()
{
	m_uniforms.Init();
	m_uniformTexCube = m_pRenderContext->CreateUniform("s_texCube", bgfx::UniformType::Sampler);
	m_uniformTexCubeIrr = m_pRenderContext->CreateUniform("s_texCubeIrr", bgfx::UniformType::Sampler);
	m_uniformTexLUT = m_pRenderContext->CreateUniform("s_texLUT", bgfx::UniformType::Sampler);

	m_lightProbeEV100 = -2.0f;
	uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
	m_lightProbeTex = m_pRenderContext->CreateTexture("skybox/bolonga_lod.dds", samplerFlags);
	m_lightProbeTexIrr = m_pRenderContext->CreateTexture("skybox/bolonga_irr.dds", samplerFlags);
	m_iblLUTTex = m_pRenderContext->CreateTexture("ibl_brdf_lut.dds");

	bgfx::ShaderHandle vsh = m_pRenderContext->CreateShader("vs_PBR_skybox.bin");
	bgfx::ShaderHandle fsh = m_pRenderContext->CreateShader("fs_PBR_skybox.bin");
	m_programSky = m_pRenderContext->CreateProgram("skybox", vsh, fsh);

	bgfx::setViewName(GetViewID(), "SkyRenderer");
}

SkyRenderer::~SkyRenderer()
{
}

void SkyRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());

	cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.Begin());
	bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
}

void SkyRenderer::Render(float deltaTime)
{
	m_uniforms.Submit();

	bgfx::setTexture(0, m_uniformTexCube, m_lightProbeTex);
	bgfx::setTexture(1, m_uniformTexCubeIrr, m_lightProbeTexIrr);
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(static_cast<float>(GetRenderTarget()->GetWidth()), static_cast<float>(GetRenderTarget()->GetHeight()), true);
	bgfx::submit(GetViewID(), m_programSky);
}

}