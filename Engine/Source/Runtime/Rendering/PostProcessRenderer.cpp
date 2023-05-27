#include "PostProcessRenderer.h"

#include "RenderContext.h"

namespace engine
{

void PostProcessRenderer::Init()
{
	m_pRenderContext->CreateUniform("s_lightingColor", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform("u_gamma", bgfx::UniformType::Vec4);
	m_pRenderContext->CreateProgram("PostProcessProgram", "vs_fullscreen.bin", "fs_PBR_postProcessing.bin");

	bgfx::setViewName(GetViewID(), "PostProcessRenderer");
}

PostProcessRenderer::~PostProcessRenderer()
{
}

void PostProcessRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	// Output to swap chain
	bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());

	cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.Begin());
}

void PostProcessRenderer::Render(float deltaTime)
{
	constexpr StringCrc sceneRenderTarget("SceneRenderTarget");

	const RenderTarget* pInputRT = m_pRenderContext->GetRenderTarget(sceneRenderTarget);
	const RenderTarget* pOutputRT = GetRenderTarget();


	bgfx::TextureHandle screenTextureHandle;
	if (pInputRT == pOutputRT)
	{
		constexpr StringCrc sceneRenderTargetBlitSRV("SceneRenderTargetBlitSRV");
		screenTextureHandle = m_pRenderContext->GetTexture(sceneRenderTargetBlitSRV);
	}
	else
	{
		screenTextureHandle = pInputRT->GetTextureHandle(0);
	}

	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);

	m_fGammaCorrection[0] = pCameraComponent->GetGammaCorrection().x();
	m_fGammaCorrection[1] = pCameraComponent->GetGammaCorrection().y();
	m_fGammaCorrection[2] = pCameraComponent->GetGammaCorrection().z();
	m_fGammaCorrection[3] = 0.0f;
	constexpr StringCrc gammaUniformName("u_gamma");
	bgfx::setUniform(m_pRenderContext->GetUniform(gammaUniformName), &m_fGammaCorrection);

	constexpr StringCrc lightingResultSampler("s_lightingColor");
	bgfx::setTexture(0, m_pRenderContext->GetUniform(lightingResultSampler), screenTextureHandle);

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(static_cast<float>(GetRenderTarget()->GetWidth()), static_cast<float>(GetRenderTarget()->GetHeight()), false);

	if (pCameraComponent->IsPostProcessEnable())
	{
		constexpr StringCrc programName("PostProcessProgram");
		bgfx::submit(GetViewID(), m_pRenderContext->GetProgram(programName));
	}
}

}