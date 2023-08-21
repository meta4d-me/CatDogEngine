#include "Bloom.h"

#include "RenderContext.h"

namespace engine
{
	void BloomRenderer::Init(){
		GetRenderContext()->CreateUniform("s_tex", bgfx::UniformType::Sampler);
		GetRenderContext()->CreateUniform("s_tex_size", bgfx::UniformType::Vec4);
		GetRenderContext()->CreateUniform("_bloomIntensity", bgfx::UniformType::Vec4);
		GetRenderContext()->CreateUniform("_luminanceThreshold", bgfx::UniformType::Vec4);
		GetRenderContext()->CreateProgram("CapTureBrightnessProgram", "vs_fullscreen.bin", "fs_captureBrightness.bin");
		GetRenderContext()->CreateProgram("DownSampleProgram", "vs_fullscreen.bin", "fs_dowmsample.bin");
		GetRenderContext()->CreateProgram("BlurVerticalProgram", "vs_fullscreen.bin", "fs_blurvertical.bin");
		GetRenderContext()->CreateProgram("BlurHorizontalProgram", "vs_fullscreen.bin", "fs_blurhorizontal.bin");
		GetRenderContext()->CreateProgram("UpSampleProgram", "vs_fullscreen.bin", "fs_upsample.bin");

		for (int i = 0; i < TEX_CHAIN_LEN; i++) m_sampleChainFB[i] = BGFX_INVALID_HANDLE;
		for (int i = 0; i < 2; i++) m_blurChainFB[i] = BGFX_INVALID_HANDLE;

		start_dowmSamplePassID = GetRenderContext()->CreateView();
		downsampleOrder.push_back(start_dowmSamplePassID);
		for (int i = 0; i < TEX_CHAIN_LEN - 2; i++) downsampleOrder.push_back(GetRenderContext()->CreateView());
		start_verticalBlurPassID = GetRenderContext()->CreateView();
		start_horizontalBlurPassID = GetRenderContext()->CreateView();
		for (int i = 0; i < 40 - 2; i++) GetRenderContext()->CreateView();
		start_upSamplePassID = GetRenderContext()->CreateView();
		for (int i = 0; i < TEX_CHAIN_LEN - 2; i++) GetRenderContext()->CreateView();
		blit_colorPassID = GetRenderContext()->CreateView();

		constexpr StringCrc bloomColor("BloomColor");
		bgfx::TextureHandle bloomColorHandle = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::RGBA32F,
			BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
		GetRenderContext()->SetTexture(bloomColor, bloomColorHandle);
	}

	BloomRenderer::~BloomRenderer()
	{
	}

	void BloomRenderer::SetEnable(bool value)
	{
		Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
		CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
		pCameraComponent->SetBloomEnable(value);
	}

	bool BloomRenderer::IsEnable() const
	{
		Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
		CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
		return pCameraComponent->GetIsBloomEnable();
	}

	void BloomRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
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
				if (bgfx::isValid(m_sampleChainFB[ii])) bgfx::destroy(m_sampleChainFB[ii]);
				if ((height >> ii) < 2 || (width >> ii) < 2) break;
				m_sampleChainFB[ii] = bgfx::createFrameBuffer(width >> ii, height >> ii, bgfx::TextureFormat::RGBA32F, tsFlags);
			}

			constexpr StringCrc bloomColor("BloomColor");
			bgfx::TextureHandle bloomColorHandle = GetRenderContext()->GetTexture(bloomColor);
			if (bgfx::isValid(bloomColorHandle)) {
				bgfx::destroy(bloomColorHandle);
			}
			bloomColorHandle = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA32F,
				BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
			GetRenderContext()->SetTexture(bloomColor, bloomColorHandle);
		}
	}

	void BloomRenderer::Render(float deltaTime)
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

		//capture
		bgfx::setViewFrameBuffer(GetViewID(), m_sampleChainFB[0]);
		bgfx::setViewRect(GetViewID(), 0, 0, width, height);
		bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.Begin());

		constexpr StringCrc luminanceThresholdUniformName("_luminanceThreshold");
		bgfx::setUniform(GetRenderContext()->GetUniform(luminanceThresholdUniformName), &pCameraComponent->GetLuminanceThreshold());

		constexpr StringCrc texSampler("s_tex");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), screenTextureHandle);

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		constexpr StringCrc CaptureBrightnessprogramName("CapTureBrightnessProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(CaptureBrightnessprogramName));

		// downsample
		int sampleTimes = int(pCameraComponent->GetBloomDownSampleTimes());
		int tempshift = 0;
		for (int i = 0; i < sampleTimes; ++i) {
			int shift = i + 1;
			if ((width >> shift) < 2 || (height >> shift) < 2) break;
			tempshift = shift;
			const float pixelSize[4] =
			{
				1.0f / (float)(width >> shift),
				1.0f / (float)(height >> shift),
				0.0f,
				0.0f,
			};

			bgfx::setViewFrameBuffer(start_dowmSamplePassID + i, m_sampleChainFB[shift]);
			bgfx::setViewRect(start_dowmSamplePassID + i, 0, 0, width >> shift, height >> shift);
			bgfx::setViewTransform(start_dowmSamplePassID + i, nullptr, orthoMatrix.Begin());

			constexpr StringCrc texelSizeName("s_tex_size");
			bgfx::setUniform(GetRenderContext()->GetUniform(texelSizeName), pixelSize);

			bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), bgfx::getTexture(m_sampleChainFB[shift - 1]));

			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
			Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

			constexpr StringCrc DownSampleprogramName("DownSampleProgram");
			bgfx::submit(start_dowmSamplePassID + i, GetRenderContext()->GetProgram(DownSampleprogramName));
		}

		if (pCameraComponent->GetIsBlurEnable() && pCameraComponent->GetBlurTimes() != 0) {
			Blur(width >> tempshift, height >> tempshift, pCameraComponent->GetBlurTimes(), pCameraComponent->GetBlurSize(), orthoMatrix, bgfx::getTexture(m_sampleChainFB[tempshift]));
		}

		// upsample
		for (int i = 0; i < sampleTimes; ++i) {
			int shift = sampleTimes - i - 1;
			if ((width >> shift) < 2 || (height >> shift) < 2) continue;
			const float pixelSize[4] =
			{
				1.0f / (float)(width >> shift),
				1.0f / (float)(height >> shift),
				0.0f,
				0.0f,
			};

			bgfx::setViewFrameBuffer(start_upSamplePassID + i, m_sampleChainFB[shift]);
			bgfx::setViewRect(start_upSamplePassID + i, 0, 0, width >> shift, height >> shift);
			bgfx::setViewTransform(start_upSamplePassID + i, nullptr, orthoMatrix.Begin());

			constexpr StringCrc texelSizeName("s_tex_size");
			bgfx::setUniform(GetRenderContext()->GetUniform(texelSizeName), pixelSize);

			constexpr StringCrc bloomIntensityName("_bloomIntensity");
			bgfx::setUniform(GetRenderContext()->GetUniform(bloomIntensityName), &pCameraComponent->GetBloomIntensity());

			if (pCameraComponent->GetIsBlurEnable() && pCameraComponent->GetBlurTimes() != 0 && i == 0) {
				bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), bgfx::getTexture(m_blurChainFB[1]));
			}
			else {
				bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), bgfx::getTexture(m_sampleChainFB[shift + 1]));
			}

			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ADD);
			Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

			constexpr StringCrc UpSampleprogramName("UpSampleProgram");
			bgfx::submit(start_upSamplePassID + i, GetRenderContext()->GetProgram(UpSampleprogramName));
		}

		constexpr StringCrc bloomColor("BloomColor");
		bgfx::TextureHandle bloomColorHandle = GetRenderContext()->GetTexture(bloomColor);
		bgfx::blit(blit_colorPassID, bloomColorHandle, 0, 0, bgfx::getTexture(m_sampleChainFB[0]));

	}

	void BloomRenderer::Blur(uint16_t width, uint16_t height, int iteration,float blursize, cd::Matrix4x4 ortho,bgfx::TextureHandle texture)
	{
		const uint64_t tsFlags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
		for (int ii = 0; ii < 2; ++ii)
		{
			if (bgfx::isValid(m_blurChainFB[ii])) bgfx::destroy(m_blurChainFB[ii]);
			m_blurChainFB[ii] = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::RGBA32F, tsFlags);
		}

		blurOrder.clear();

		uint16_t vertical = start_verticalBlurPassID;
		uint16_t horizontal = start_horizontalBlurPassID;
		blurOrder.push_back(vertical);
		blurOrder.push_back(horizontal);
		for (int i = 0; i < iteration; i++) {
			float pixelSize[4] =
			{
				1.0f / (float)(width),
				1.0f / (float)(height),
				1.0f + blursize,
				1.0f,
			};

			bgfx::setViewFrameBuffer(vertical, m_blurChainFB[0]);
			bgfx::setViewRect(vertical, 0, 0, width, height);
			bgfx::setViewTransform(vertical, nullptr, ortho.Begin());

			constexpr StringCrc texelSizeName("s_tex_size");
			bgfx::setUniform(GetRenderContext()->GetUniform(texelSizeName), pixelSize);

			constexpr StringCrc texSampler("s_tex");
			bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), i == 0 ? texture : bgfx::getTexture(m_blurChainFB[1]));

			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
			Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

			constexpr StringCrc BlurHorizontalprogramName("BlurHorizontalProgram");
			bgfx::submit(vertical, GetRenderContext()->GetProgram(BlurHorizontalprogramName));

			// vertical
			bgfx::setViewFrameBuffer(horizontal, m_blurChainFB[1]);
			bgfx::setViewRect(horizontal, 0, 0, width, height);
			bgfx::setViewTransform(horizontal, nullptr, ortho.Begin());

			bgfx::setUniform(GetRenderContext()->GetUniform(texelSizeName), pixelSize);

			bgfx::setTexture(0, GetRenderContext()->GetUniform(texSampler), bgfx::getTexture(m_blurChainFB[0]));

			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
			Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

			constexpr StringCrc BlurVerticalprogramName("BlurVerticalProgram");
			bgfx::submit(horizontal, GetRenderContext()->GetProgram(BlurVerticalprogramName));

			vertical += 2;
			horizontal += 2;
			blurOrder.push_back(vertical);
			blurOrder.push_back(horizontal);
		}
	}

}
