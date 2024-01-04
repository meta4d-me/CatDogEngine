#include "PostProcessRenderer.h"

#include "Rendering/RenderContext.h"

namespace engine
{

namespace
{

constexpr const char *PostProcessProgram = "PostProcessProgram";
constexpr StringCrc PostProcessProgramCrc = StringCrc(PostProcessProgram);

}

void PostProcessRenderer::Init()
{
	GetRenderContext()->RegisterShaderProgram(PostProcessProgramCrc, { "vs_fullscreen", "fs_PBR_postProcessing" });

	bgfx::setViewName(GetViewID(), "PostProcessRenderer");
}

void PostProcessRenderer::Warmup()
{
	GetRenderContext()->CreateUniform("s_lightingColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("u_postProcessingParams", bgfx::UniformType::Vec4);

	GetRenderContext()->UploadShaderProgram(PostProcessProgram);
}

void PostProcessRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();

	cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.begin());
}

void PostProcessRenderer::Render(float deltaTime)
{
	constexpr StringCrc sceneRenderTarget("SceneRenderTarget");

	const RenderTarget* pInputRT = GetRenderContext()->GetRenderTarget(sceneRenderTarget);
	const RenderTarget* pOutputRT = GetRenderTarget();

	bgfx::TextureHandle screenTextureHandle;
	if (pInputRT == pOutputRT)
	{
		constexpr StringCrc sceneRenderTargetBlitSRV("SceneRenderTargetBlitSRV");
		screenTextureHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitSRV);
	}
	else
	{
		screenTextureHandle = pInputRT->GetTextureHandle(0);
	}

	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);

	constexpr StringCrc paramUniformName("u_postProcessingParams");
	static cd::Vec4f postProcessingParams;
	postProcessingParams[0] = pCameraComponent->GetExposure();
	postProcessingParams[1] = static_cast<float>(pCameraComponent->GetToneMappingMode());
	postProcessingParams[2] = pCameraComponent->GetGammaCorrection();
	bgfx::setUniform(GetRenderContext()->GetUniform(paramUniformName), &postProcessingParams);

	constexpr StringCrc lightingResultSampler("s_lightingColor");
	bgfx::setTexture(0, GetRenderContext()->GetUniform(lightingResultSampler), screenTextureHandle);

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

	GetRenderContext()->Submit(GetViewID(), PostProcessProgram);
}

}