#include "SkyRenderer.h"

#include "ECWorld/SceneWorld.h"
#include "RenderContext.h"

namespace engine
{

void SkyRenderer::Init()
{
	m_pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	m_pRenderContext->CreateUniform("s_texLUT", bgfx::UniformType::Sampler);
	constexpr uint16_t sampleFlag = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
	m_pRenderContext->CreateTexture(m_pSkyComponent->GetIrradianceTexturePath().c_str(), sampleFlag);
	m_pRenderContext->CreateTexture(m_pSkyComponent->GetRadianceTexturePath().c_str(), sampleFlag);

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

}

}