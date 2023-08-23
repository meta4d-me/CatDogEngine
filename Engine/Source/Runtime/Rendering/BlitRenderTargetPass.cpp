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

	constexpr StringCrc sceneRenderTargetBlitSRV("SceneRenderTargetBlitSRV");
	bgfx::TextureHandle blitTargetSRVHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitSRV);
	bool buildSRV = false;
	if (bgfx::isValid(blitTargetSRVHandle))
	{
		if (pSceneRT->GetWidth() != m_blitTextureWidth ||
			pSceneRT->GetHeight() != m_blitTextureHeight)
		{
			bgfx::destroy(blitTargetSRVHandle);
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
	}

	bgfx::blit(GetViewID(), blitTargetSRVHandle, 0, 0, sceneColorTextureHandle);
}

}