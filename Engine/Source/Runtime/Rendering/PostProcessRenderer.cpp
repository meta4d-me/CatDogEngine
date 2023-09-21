#include "PostProcessRenderer.h"

#include "Rendering/RenderContext.h"
#include "Rendering/ShaderVariantCollections.h"

namespace engine
{

void PostProcessRenderer::Init()
{
	GetRenderContext()->RegisterNonUberShader("PostProcessProgram", { "vs_fullscreen","fs_PBR_postProcessing" });

	bgfx::setViewName(GetViewID(), "PostProcessRenderer");
}

void PostProcessRenderer::Warmup()
{
	GetRenderContext()->CreateUniform("u_gamma", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("s_lightingColor", bgfx::UniformType::Sampler);

	GetRenderContext()->UploadNonUberShader("PostProcessProgram");
}

void PostProcessRenderer::SetEnable(bool value)
{
	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
	pCameraComponent->SetToneMappingEnable(value);
}

bool PostProcessRenderer::IsEnable() const
{
	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
	return pCameraComponent->GetIsToneMappingEnable();
}

void PostProcessRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();

	cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.Begin());
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

	constexpr StringCrc gammaUniformName("u_gamma");
	bgfx::setUniform(GetRenderContext()->GetUniform(gammaUniformName), &pCameraComponent->GetGammaCorrection());

	constexpr StringCrc lightingResultSampler("s_lightingColor");
	bgfx::setTexture(0, GetRenderContext()->GetUniform(lightingResultSampler), screenTextureHandle);

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

	GetRenderContext()->Submit(GetViewID(), "PostProcessProgram");
}

}