#include "BlitRenderTargetPass.h"

#include "RenderContext.h"

namespace engine
{

void BlitRenderTargetPass::Init()
{
	bgfx::setViewName(GetViewID(), "BlitRenderTargetPass");
}

BlitRenderTargetPass::~BlitRenderTargetPass()
{
}

void BlitRenderTargetPass::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
}

void BlitRenderTargetPass::Render(float deltaTime)
{
	constexpr StringCrc sceneRenderTarget("SceneRenderTarget");
	const RenderTarget* pSceneRT = GetRenderContext()->GetRenderTarget(sceneRenderTarget);
	bgfx::TextureHandle sceneColorTextureHandle = pSceneRT->GetTextureHandle(0);
	bgfx::TextureHandle emissColorTextureHandle = pSceneRT->GetTextureHandle(1);
	bgfx::TextureHandle viewPositiontextureHandle = pSceneRT->GetTextureHandle(2);
	bgfx::TextureHandle viewNormaltextureHandle = pSceneRT->GetTextureHandle(3);
	bgfx::TextureHandle depthColorTextureHandle = pSceneRT->GetTextureHandle(4);

	constexpr StringCrc sceneRenderTargetBlitSRV("SceneRenderTargetBlitSRV");
	constexpr StringCrc sceneRenderTargetBlitEmissColor("SceneRenderTargetBlitEmissColor");
	constexpr StringCrc sceneRenderTargetBlitViewPosition("SceneRenderTargetBlitViewPosition");
	constexpr StringCrc sceneRenderTargetBlitViewNormal("SceneRenderTargetBlitViewNormal");
	constexpr StringCrc sceneRenderTargetBlitDepthColor("SceneRenderTargetBlitDepthColor");

	bgfx::TextureHandle blitTargetSRVHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitSRV);
	bgfx::TextureHandle blitTargetEmissColorHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitEmissColor);
	bgfx::TextureHandle blitTargetViewPositionHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitViewPosition);
	bgfx::TextureHandle blitTargetViewNormalHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitViewNormal);
	bgfx::TextureHandle blitTargetDepthColorHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitDepthColor);

	bool buildSRV = false;
	if (bgfx::isValid(blitTargetSRVHandle))
	{
		if (pSceneRT->GetWidth() != m_blitTextureWidth ||
			pSceneRT->GetHeight() != m_blitTextureHeight)
		{
			bgfx::destroy(blitTargetSRVHandle);
			bgfx::destroy(blitTargetEmissColorHandle);
			bgfx::destroy(blitTargetViewPositionHandle);
			bgfx::destroy(blitTargetViewNormalHandle);
			bgfx::destroy(blitTargetDepthColorHandle);
			buildSRV = true;
		}
	}
	else
	{
		buildSRV = true;
	}

	if (buildSRV)
	{
		m_blitTextureWidth = pSceneRT->GetWidth();
		m_blitTextureHeight = pSceneRT->GetHeight();
		blitTargetSRVHandle = bgfx::createTexture2D(m_blitTextureWidth, m_blitTextureHeight, false, 1, bgfx::TextureFormat::RGBA32F,
			BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		GetRenderContext()->SetTexture(sceneRenderTargetBlitSRV, blitTargetSRVHandle);

		blitTargetEmissColorHandle = bgfx::createTexture2D(m_blitTextureWidth, m_blitTextureHeight, false, 1, bgfx::TextureFormat::RGBA32F,
			BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		GetRenderContext()->SetTexture(sceneRenderTargetBlitEmissColor, blitTargetEmissColorHandle);

		blitTargetViewPositionHandle = bgfx::createTexture2D(m_blitTextureWidth, m_blitTextureHeight, false, 1, bgfx::TextureFormat::RGBA32F,
			BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		GetRenderContext()->SetTexture(sceneRenderTargetBlitViewPosition, blitTargetViewPositionHandle);

		blitTargetViewNormalHandle = bgfx::createTexture2D(m_blitTextureWidth, m_blitTextureHeight, false, 1, bgfx::TextureFormat::RGBA32F,
			BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		GetRenderContext()->SetTexture(sceneRenderTargetBlitViewNormal, blitTargetViewNormalHandle);

		blitTargetDepthColorHandle = bgfx::createTexture2D(m_blitTextureWidth, m_blitTextureHeight, false, 1, bgfx::TextureFormat::D32F,
			BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		GetRenderContext()->SetTexture(sceneRenderTargetBlitDepthColor, blitTargetDepthColorHandle);
	}

	bgfx::blit(GetViewID(), blitTargetSRVHandle, 0, 0, sceneColorTextureHandle);
	bgfx::blit(GetViewID(), blitTargetEmissColorHandle, 0, 0, emissColorTextureHandle);
	bgfx::blit(GetViewID(), blitTargetViewPositionHandle, 0, 0, viewPositiontextureHandle);
	bgfx::blit(GetViewID(), blitTargetViewNormalHandle, 0, 0, viewNormaltextureHandle);
	bgfx::blit(GetViewID(), blitTargetDepthColorHandle, 0, 0, depthColorTextureHandle);
}

}