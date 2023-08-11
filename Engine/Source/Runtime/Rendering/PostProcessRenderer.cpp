#include "PostProcessRenderer.h"

#include "RenderContext.h"

namespace engine
{

void PostProcessRenderer::Init()
{
	GetRenderContext()->CreateUniform("s_lightingColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("s_bloomColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("u_gamma", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateProgram("PostProcessProgram", "vs_fullscreen.bin", "fs_PBR_postProcessing.bin");

	bgfx::setViewName(GetViewID(), "PostProcessRenderer");

	GetRenderContext()->CreateUniform("u_pixelSize", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("s_tex", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("u_intensity", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("u_range", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateProgram("CaptureBrightColorProgram", "vs_fullscreen.bin", "fs_brightcolor.bin");
	GetRenderContext()->CreateProgram("DownSampleProgram", "vs_fullscreen.bin", "fs_downsample.bin");
	GetRenderContext()->CreateProgram("UpSampleProgram", "vs_fullscreen.bin", "fs_upsample.bin");

	pre_width = 0;
	pre_height = 0;

	capturebrightness_id = GetRenderContext()->GetCurrentViewID() + 1;
	start_downsample_id = capturebrightness_id + 1;
	start_upsample_id = start_downsample_id + TEX_CHAIN_LEN;
	for (int ii = 0; ii < TEX_CHAIN_LEN; ++ii) 	m_texChainFb[ii] = BGFX_INVALID_HANDLE;
}

PostProcessRenderer::~PostProcessRenderer()
{
}

void PostProcessRenderer::SetEnable(bool value)
{
	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
	pCameraComponent->SetPostProcessEnable(value);
}

bool PostProcessRenderer::IsEnable() const
{
	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
	return pCameraComponent->IsPostProcessEnable();
}

void PostProcessRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{

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

	cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

	uint16_t width = 0;
	uint16_t height = 0;
	GetWidthHeight(width, height);

	if (width != pre_width || height != pre_height) {
		pre_width = width;
		pre_height = height;

		for (int ii = 0; ii < TEX_CHAIN_LEN; ++ii)
		{
			if (bgfx::isValid(m_texChainFb[ii])) bgfx::destroy(m_texChainFb[ii]);
			
			m_texChainFb[ii] = bgfx::createFrameBuffer(
				(uint16_t)(width >> ii)
				, (uint16_t)(height >> ii)
				, bgfx::TextureFormat::RGBA32F
				, 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
			);
		}
	}

	// capture brightness
	bgfx::setViewFrameBuffer(capturebrightness_id, m_texChainFb[0]);
	bgfx::setViewRect(capturebrightness_id, 0, 0, width, height);
	bgfx::setViewTransform(capturebrightness_id, nullptr, orthoMatrix.Begin());

	constexpr StringCrc texUniformName("s_tex");
	bgfx::setTexture(0, GetRenderContext()->GetUniform(texUniformName), screenTextureHandle);

	bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);

	Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

	constexpr StringCrc capturebrightnessprogramName("CaptureBrightColorProgram");
	bgfx::submit(capturebrightness_id, GetRenderContext()->GetProgram(capturebrightnessprogramName));

	// downsample
	for (uint16_t ii = 0; ii < TEX_CHAIN_LEN - 1; ++ii)
	{
		const uint16_t shift = ii + 1;
		const float pixelSize[4] =
		{
			1.0f / (float)(width >> shift),
			1.0f / (float)(height >> shift),
			0.0f,
			0.0f,
		};

		bgfx::setViewFrameBuffer(start_downsample_id + ii, m_texChainFb[ii + 1]);
		bgfx::setViewRect(start_downsample_id + ii, 0, 0, width, height);
		bgfx::setViewTransform(start_downsample_id + ii, nullptr, orthoMatrix.Begin());

		constexpr StringCrc pixelSizeUniformName("u_pixelSize");
		bgfx::setUniform(GetRenderContext()->GetUniform(pixelSizeUniformName), pixelSize);

		constexpr StringCrc texUniformName("s_tex");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(texUniformName), bgfx::getTexture(m_texChainFb[ii]));

		bgfx::setState(0| BGFX_STATE_WRITE_RGB| BGFX_STATE_WRITE_A);

		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		constexpr StringCrc downsampleprogramName("DownSampleProgram");
		bgfx::submit(start_downsample_id + ii, GetRenderContext()->GetProgram(downsampleprogramName));
	}

	// upsample
	for (uint16_t ii = 0; ii < TEX_CHAIN_LEN - 1; ++ii)
	{
		const uint16_t shift = TEX_CHAIN_LEN - 2 - ii;
		const float pixelSize[4] =
		{
			1.0f / (float)(width >> shift),
			1.0f / (float)(height >> shift),
			0.0f,
			0.0f,
		};

		bgfx::setViewFrameBuffer(start_upsample_id + ii, m_texChainFb[TEX_CHAIN_LEN - 2 - ii]);
		bgfx::setViewRect(start_upsample_id + ii, 0, 0, width,height);
		bgfx::setViewTransform(start_upsample_id + ii, nullptr, orthoMatrix.Begin());

		constexpr StringCrc pixelSizeUniformName("u_pixelSize");
		bgfx::setUniform(GetRenderContext()->GetUniform(pixelSizeUniformName), pixelSize);

		const float intensity[4] = { pCameraComponent->GetBloomIntensity(),0.0f};
		constexpr StringCrc intensityUniformName("u_intensity");
		bgfx::setUniform(GetRenderContext()->GetUniform(intensityUniformName), intensity);

		const float range[4] = { pCameraComponent->GetBloomRange(),0.0f };
		constexpr StringCrc rangeUniformName("u_range");
		bgfx::setUniform(GetRenderContext()->GetUniform(rangeUniformName), range);

		constexpr StringCrc texUniformName("s_tex");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(texUniformName), bgfx::getTexture(m_texChainFb[TEX_CHAIN_LEN - 1 - ii]));

		bgfx::setState(0| BGFX_STATE_WRITE_RGB| BGFX_STATE_WRITE_A);

		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		constexpr StringCrc upsampleprogramName("UpSampleProgram");
		bgfx::submit(start_upsample_id + ii, GetRenderContext()->GetProgram(upsampleprogramName));
	}

	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.Begin());

	constexpr StringCrc gammaUniformName("u_gamma");
	bgfx::setUniform(GetRenderContext()->GetUniform(gammaUniformName), &pCameraComponent->GetGammaCorrection());

	constexpr StringCrc lightingResultSampler("s_lightingColor");
	bgfx::setTexture(0, GetRenderContext()->GetUniform(lightingResultSampler), screenTextureHandle);

	constexpr StringCrc bloomColorSampler("s_bloomColor");
	bgfx::setTexture(1, GetRenderContext()->GetUniform(bloomColorSampler), bgfx::getTexture(m_texChainFb[0]));

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

	constexpr StringCrc programName("PostProcessProgram");
	bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(programName));
}

}