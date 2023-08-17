#include "PostProcessRenderer.h"

#include "RenderContext.h"

namespace engine
{

void PostProcessRenderer::Init()
{
	GetRenderContext()->CreateUniform("s_lightingColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("u_gamma", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateProgram("PostProcessProgram", "vs_fullscreen.bin", "fs_PBR_postProcessing.bin");

	bgfx::setViewName(GetViewID(), "PostProcessRenderer");

	GetRenderContext()->CreateUniform("s_bloomColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("s_tex", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("s_tex_size", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("_bloomIntensity", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("_luminanceThreshold", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("_horizontal", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateProgram("CaptureBrightnessProgram", "vs_fullscreen.bin", "fs_captureBrightness.bin");
	GetRenderContext()->CreateProgram("DownSampleProgram", "vs_fullscreen.bin", "fs_dowmsample.bin");
	GetRenderContext()->CreateProgram("UpSampleProgram", "vs_fullscreen.bin", "fs_upsample.bin");
	GetRenderContext()->CreateProgram("BlurProgram", "vs_fullscreen.bin", "fs_blur.bin");

	width = 0;
	height = 0;
	captureBrightnessPassID = GetRenderContext()->GetCurrentViewCount() + 1;
	start_DownSamplePassID = captureBrightnessPassID + 1;
	blurPassID = start_DownSamplePassID + TEX_CHAIN_LEN;
	start_UpSamplePassID = blurPassID + 2;
	for (int i = 0; i < TEX_CHAIN_LEN; ++i) m_bloomFB[i] = BGFX_INVALID_HANDLE;
	m_blurFB = BGFX_INVALID_HANDLE;
}

PostProcessRenderer::~PostProcessRenderer()
{
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
	return pCameraComponent->IsToneMappingEnable();
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

	cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
	 
	if(pCameraComponent->GetIsBloomEnable()) Bloom(orthoMatrix, pCameraComponent, screenTextureHandle);

	UpdateViewRenderTarget();

	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.Begin());

	constexpr StringCrc gammaUniformName("u_gamma");
	bgfx::setUniform(GetRenderContext()->GetUniform(gammaUniformName), &pCameraComponent->GetGammaCorrection());

	constexpr StringCrc lightingResultSampler("s_lightingColor");
	bgfx::setTexture(0, GetRenderContext()->GetUniform(lightingResultSampler), screenTextureHandle);

	if (pCameraComponent->GetIsBloomEnable()) {
		constexpr StringCrc bloomColorSampler("s_bloomColor");
		bgfx::setTexture(1, GetRenderContext()->GetUniform(bloomColorSampler), bgfx::getTexture(m_bloomFB[0]));
	}

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

	constexpr StringCrc programName("PostProcessProgram");
	bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(programName));
}

void PostProcessRenderer::Bloom(cd::Matrix4x4 ortho, CameraComponent* pCameraComponent, bgfx::TextureHandle screenTextureHandle)
{
	uint16_t tempW = 0;
	uint16_t tempH = 0;
	if (m_pRenderTarget)
	{
		tempW = GetRenderTarget()->GetWidth();
		tempH = GetRenderTarget()->GetHeight();
	}
	else
	{
		assert(GetRenderContext());
		tempW = GetRenderContext()->GetBackBufferWidth();
		tempH = GetRenderContext()->GetBackBufferHeight();
	}

	if (width != tempW || height != tempH) {
		width = tempW;
		height = tempH;

		const uint64_t tsFlags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
		for (int ii = 0; ii < TEX_CHAIN_LEN; ++ii)
		{
			if (bgfx::isValid(m_bloomFB[ii])) bgfx::destroy(m_bloomFB[ii]);
			if ((height >> ii) < 2 || (width >> ii) < 2) break;
			m_bloomFB[ii] = bgfx::createFrameBuffer(width >> ii, height >> ii, bgfx::TextureFormat::RGBA32F, tsFlags);
		}
	}

	// CaptureBrightness
	bgfx::setViewFrameBuffer(captureBrightnessPassID, m_bloomFB[0]);
	bgfx::setViewRect(captureBrightnessPassID, 0, 0, width, height);
	bgfx::setViewTransform(captureBrightnessPassID, nullptr, ortho.Begin());

	constexpr StringCrc gammaUniformName("_luminanceThreshold");
	bgfx::setUniform(GetRenderContext()->GetUniform(gammaUniformName), &pCameraComponent->GetLuminanceThreshold());

	constexpr StringCrc texSampler("s_tex");
	bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), screenTextureHandle);

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

	constexpr StringCrc CaptureBrightnessprogramName("CaptureBrightnessProgram");
	bgfx::submit(captureBrightnessPassID, GetRenderContext()->GetProgram(CaptureBrightnessprogramName));

	// DownSample
	int sampleTimes = int(pCameraComponent->GetBloomDownSampleTimes());
	for (int ii = 0; ii < sampleTimes; ++ii) {
		int shift = ii + 1;
		if ((tempH >> shift) < 2 || (tempW >> shift) < 2) break;

		const float pixelSize[4] =
		{
			1.0f / (float)(tempW >> shift),
			1.0f / (float)(tempH >> shift),
			0.0f,
			0.0f,
		};

		bgfx::setViewFrameBuffer(start_DownSamplePassID + ii, m_bloomFB[shift]);
		bgfx::setViewRect(start_DownSamplePassID + ii, 0, 0, tempW >> shift, tempH >> shift);
		bgfx::setViewTransform(start_DownSamplePassID + ii, nullptr, ortho.Begin());

		constexpr StringCrc texelSizeName("s_tex_size");
		bgfx::setUniform(GetRenderContext()->GetUniform(texelSizeName), pixelSize);

		bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), bgfx::getTexture(m_bloomFB[shift - 1]));

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		constexpr StringCrc DownSampleprogramName("DownSampleProgram");
		bgfx::submit(start_DownSamplePassID + ii, GetRenderContext()->GetProgram(DownSampleprogramName));

		if(pCameraComponent->GetIsBlurEnable()) Blur(tempW >> shift, tempH >> shift, ortho, pCameraComponent, shift);
	}

	// UpSample
	for (int ii = 0; ii < sampleTimes; ++ii) {
		int shift = sampleTimes - ii - 1;
		const float pixelSize[4] =
		{
			1.0f / (float)(tempW >> shift),
			1.0f / (float)(tempH >> shift),
			0.0f,
			0.0f,
		};

		bgfx::setViewFrameBuffer(start_UpSamplePassID + ii, m_bloomFB[shift]);
		bgfx::setViewRect(start_UpSamplePassID + ii, 0, 0, tempW >> shift, tempH >> shift);
		bgfx::setViewTransform(start_UpSamplePassID + ii, nullptr, ortho.Begin());

		constexpr StringCrc texelSizeName("s_tex_size");
		bgfx::setUniform(GetRenderContext()->GetUniform(texelSizeName), pixelSize);

		constexpr StringCrc bloomIntensityName("_bloomIntensity");
		bgfx::setUniform(GetRenderContext()->GetUniform(bloomIntensityName), &pCameraComponent->GetBloomIntensity());

		bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), bgfx::getTexture(m_bloomFB[shift + 1]));

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ADD);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		constexpr StringCrc UpSampleprogramName("UpSampleProgram");
		bgfx::submit(start_UpSamplePassID + ii, GetRenderContext()->GetProgram(UpSampleprogramName));
	}
}

void PostProcessRenderer::Blur(uint16_t width, uint16_t height, cd::Matrix4x4 ortho, CameraComponent* pCameraComponent, int index)
{
	const uint64_t tsFlags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	if (pCameraComponent->GetBlurTimes() > 0.0f) {
		if (bgfx::isValid(m_blurFB)) bgfx::destroy(m_blurFB);
		m_blurFB = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::RGBA32F, tsFlags);
	}

	for (int ii = 0; ii < int(pCameraComponent->GetBlurTimes()); ++ii)
	{
		float pixelSize[4] =
		{
			1.0f / (float)(width),
			1.0f / (float)(height),
			1.0f,
			1.0f + ii * pCameraComponent->GetBlurSize()
		};

		bgfx::setViewFrameBuffer(blurPassID, m_blurFB);
		bgfx::setViewRect(blurPassID, 0, 0, width, height);
		bgfx::setViewTransform(blurPassID, nullptr, ortho.Begin());

		pixelSize[2] = 1.0f;
		constexpr StringCrc texelSizeName("s_tex_size");
		bgfx::setUniform(GetRenderContext()->GetUniform(texelSizeName), pixelSize);

		constexpr StringCrc texSampler("s_tex");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), bgfx::getTexture(m_bloomFB[index]));

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		constexpr StringCrc BlurprogramName("BlurProgram");
		bgfx::submit(blurPassID, GetRenderContext()->GetProgram(BlurprogramName));

		// vertical
		bgfx::setViewFrameBuffer(blurPassID + 1, m_bloomFB[index]);
		bgfx::setViewRect(blurPassID + 1, 0, 0, width, height);
		bgfx::setViewTransform(blurPassID + 1, nullptr, ortho.Begin());

		pixelSize[2] = 0.0f;
		bgfx::setUniform(GetRenderContext()->GetUniform(texelSizeName), pixelSize);

		bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler),  bgfx::getTexture(m_blurFB));

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		bgfx::submit(blurPassID + 1, GetRenderContext()->GetProgram(BlurprogramName));
	}
}

}