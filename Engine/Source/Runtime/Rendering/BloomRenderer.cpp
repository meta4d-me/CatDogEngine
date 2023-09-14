#include "BloomRenderer.h"

#include "Rendering/RenderContext.h"
#include "Rendering/ShaderVariantCollections.h"

namespace engine
{
	void BloomRenderer::Init()
	{
		GetShaderVariantCollections()->RegisterPragram("CapTureBrightnessProgram", { "vs_fullscreen", "fs_captureBrightness" });
		GetShaderVariantCollections()->RegisterPragram("DownSampleProgram", { "vs_fullscreen", "fs_dowmsample" });
		GetShaderVariantCollections()->RegisterPragram("BlurVerticalProgram", { "vs_fullscreen", "fs_blurvertical" });
		GetShaderVariantCollections()->RegisterPragram("BlurHorizontalProgram", { "vs_fullscreen", "fs_blurhorizontal" });
		GetShaderVariantCollections()->RegisterPragram("UpSampleProgram", { "vs_fullscreen", "fs_upsample" });
		GetShaderVariantCollections()->RegisterPragram("KawaseBlurProgram", { "vs_fullscreen", "fs_kawaseblur" });
		GetShaderVariantCollections()->RegisterPragram("CombineProgram", { "vs_fullscreen", "fs_bloom" });
	
		bgfx::setViewName(GetViewID(), "BloomRenderer");
	}

	void BloomRenderer::Submit()
	{
		for (int i = 0; i < TEX_CHAIN_LEN; i++)
		{
			m_sampleChainFB[i] = BGFX_INVALID_HANDLE;
		}
		for (int i = 0; i < 2; i++)
		{
			m_blurChainFB[i] = BGFX_INVALID_HANDLE;
		}
		m_combineFB = BGFX_INVALID_HANDLE;
		m_startDowmSamplePassID = GetRenderContext()->CreateView();
		for (int i = 0; i < TEX_CHAIN_LEN - 2; i++)
		{
			GetRenderContext()->CreateView();
		}
		m_startVerticalBlurPassID = GetRenderContext()->CreateView();
		m_startHorizontalBlurPassID = GetRenderContext()->CreateView();
		for (int i = 0; i < 40 - 2; i++)
		{
			GetRenderContext()->CreateView();
		}
		m_startUpSamplePassID = GetRenderContext()->CreateView();
		for (int i = 0; i < TEX_CHAIN_LEN - 2; i++)
		{
			GetRenderContext()->CreateView();
		}
		m_CombinePassID = GetRenderContext()->CreateView();
		m_blitColorPassID = GetRenderContext()->CreateView();

		GetRenderContext()->CreateUniform("s_texture", bgfx::UniformType::Sampler);
		GetRenderContext()->CreateUniform("s_bloom", bgfx::UniformType::Sampler);
		GetRenderContext()->CreateUniform("s_lightingColor", bgfx::UniformType::Sampler);
		GetRenderContext()->CreateUniform("u_textureSize", bgfx::UniformType::Vec4);
		GetRenderContext()->CreateUniform("u_bloomIntensity", bgfx::UniformType::Vec4);
		GetRenderContext()->CreateUniform("u_luminanceThreshold", bgfx::UniformType::Vec4);

		GetRenderContext()->CreateProgram("CapTureBrightnessProgram", "vs_fullscreen.bin", "fs_captureBrightness.bin");
		GetRenderContext()->CreateProgram("DownSampleProgram", "vs_fullscreen.bin", "fs_dowmsample.bin");
		GetRenderContext()->CreateProgram("BlurVerticalProgram", "vs_fullscreen.bin", "fs_blurvertical.bin");
		GetRenderContext()->CreateProgram("BlurHorizontalProgram", "vs_fullscreen.bin", "fs_blurhorizontal.bin");
		GetRenderContext()->CreateProgram("UpSampleProgram", "vs_fullscreen.bin", "fs_upsample.bin");
		GetRenderContext()->CreateProgram("KawaseBlurProgram", "vs_fullscreen.bin", "fs_kawaseblur.bin");
		GetRenderContext()->CreateProgram("CombineProgram", "vs_fullscreen.bin", "fs_bloom.bin");
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

		if (m_width != tempW || m_height != tempH)
		{
			m_width = tempW;
			m_height = tempH;

			constexpr uint64_t tsFlags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
			for (int ii = 0; ii < TEX_CHAIN_LEN; ++ii) 
			{
				if (bgfx::isValid(m_sampleChainFB[ii]))
				{
					bgfx::destroy(m_sampleChainFB[ii]);
				}
				if ((m_height >> ii) < 2 || (m_width >> ii) < 2)
				{
					break;
				}
				m_sampleChainFB[ii] = bgfx::createFrameBuffer(m_width >> ii, m_height >> ii, bgfx::TextureFormat::RGBA32F, tsFlags);
			}

			m_combineFB = bgfx::createFrameBuffer(m_width, m_height, bgfx::TextureFormat::RGBA32F, tsFlags);
		}
	}

	void BloomRenderer::Render(float deltaTime)
	{
		constexpr StringCrc sceneRenderTarget("SceneRenderTarget");

		const RenderTarget* pInputRT = GetRenderContext()->GetRenderTarget(sceneRenderTarget);
		const RenderTarget* pOutputRT = GetRenderTarget();

		bgfx::TextureHandle screenEmissColorTextureHandle;
		bgfx::TextureHandle screenTextureHandle;
		if (pInputRT == pOutputRT)
		{
			constexpr StringCrc sceneRenderTargetBlitSRV("SceneRenderTargetBlitSRV");
			screenTextureHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitSRV);

			constexpr StringCrc sceneRenderTargetBlitEmissColor("SceneRenderTargetBlitEmissColor");
			screenEmissColorTextureHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitEmissColor);
		}
		else
		{
			screenTextureHandle = pInputRT->GetTextureHandle(0);
			screenEmissColorTextureHandle = pInputRT->GetTextureHandle(1);
		}

		Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
		CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);

		cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

		// capture
		bgfx::setViewFrameBuffer(GetViewID(), m_sampleChainFB[0]);
		bgfx::setViewRect(GetViewID(), 0, 0, m_width, m_height);
		bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.Begin());

		constexpr StringCrc luminanceThresholdUniformName("u_luminanceThreshold");
		bgfx::setUniform(GetRenderContext()->GetUniform(luminanceThresholdUniformName), &pCameraComponent->GetLuminanceThreshold());

		constexpr StringCrc textureSampler("s_texture");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), screenEmissColorTextureHandle);

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		constexpr StringCrc CaptureBrightnessprogramName("CapTureBrightnessProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(CaptureBrightnessprogramName));

		// downsample
		int sampleTimes = int(pCameraComponent->GetBloomDownSampleTimes());
		int tempshift = 0;
		for (int i = 0; i < sampleTimes; ++i)
		{
			int shift = i + 1;
			if ((m_width >> shift) < 2 || (m_height >> shift) < 2)
			{
				break;
			}

			tempshift = shift;
			const float pixelSize[4] =
			{
				1.0f / static_cast<float>(m_width >> shift),
				1.0f / static_cast<float>(m_height >> shift),
				0.0f,
				0.0f,
			};

			bgfx::setViewFrameBuffer(m_startDowmSamplePassID + i, m_sampleChainFB[shift]);
			bgfx::setViewRect(m_startDowmSamplePassID + i, 0, 0, m_width >> shift, m_height >> shift);
			bgfx::setViewTransform(m_startDowmSamplePassID + i, nullptr, orthoMatrix.Begin());

			constexpr StringCrc textureSizeUniformName("u_textureSize");
			bgfx::setUniform(GetRenderContext()->GetUniform(textureSizeUniformName), pixelSize);

			bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), bgfx::getTexture(m_sampleChainFB[shift - 1]));

			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
			Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

			constexpr StringCrc DownSampleprogramName("DownSampleProgram");
			bgfx::submit(m_startDowmSamplePassID + i, GetRenderContext()->GetProgram(DownSampleprogramName));
		}

		if (pCameraComponent->GetIsBlurEnable() && pCameraComponent->GetBlurTimes() != 0)
		{
			Blur(m_width >> tempshift, m_height >> tempshift, pCameraComponent->GetBlurTimes(), pCameraComponent->GetBlurSize(),pCameraComponent->GetBlurScaling(), orthoMatrix, bgfx::getTexture(m_sampleChainFB[tempshift]));
		}

		// upsample
		for (int i = 0; i < sampleTimes; ++i)
		{
			int shift = sampleTimes - i - 1;
			if ((m_width >> shift) < 2 || (m_height >> shift) < 2)
			{
				continue;
			}

			const float pixelSize[4] =
			{
				1.0f / static_cast<float>(m_width >> shift),
				1.0f / static_cast<float>(m_height >> shift),
				0.0f,
				0.0f,
			};

			bgfx::setViewFrameBuffer(m_startUpSamplePassID + i, m_sampleChainFB[shift]);
			bgfx::setViewRect(m_startUpSamplePassID + i, 0, 0, m_width >> shift, m_height >> shift);
			bgfx::setViewTransform(m_startUpSamplePassID + i, nullptr, orthoMatrix.Begin());

			constexpr StringCrc textureSizeUniformName("u_textureSize");
			bgfx::setUniform(GetRenderContext()->GetUniform(textureSizeUniformName), pixelSize);

			constexpr StringCrc bloomIntensityUniformName("u_bloomIntensity");
			bgfx::setUniform(GetRenderContext()->GetUniform(bloomIntensityUniformName), &pCameraComponent->GetBloomIntensity());

			if (pCameraComponent->GetIsBlurEnable() && pCameraComponent->GetBlurTimes() != 0 && 0 == i) 
			{
				bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), bgfx::getTexture(m_blurChainFB[1]));
			}
			else 
			{
				bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), bgfx::getTexture(m_sampleChainFB[shift + 1]));
			}

			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ADD);
			Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

			constexpr StringCrc UpSampleprogramName("UpSampleProgram");
			bgfx::submit(m_startUpSamplePassID + i, GetRenderContext()->GetProgram(UpSampleprogramName));
		}

		// combine 
		bgfx::setViewFrameBuffer(m_CombinePassID, m_combineFB);
		bgfx::setViewRect(m_CombinePassID, 0, 0, m_width, m_height);
		bgfx::setViewTransform(m_CombinePassID, nullptr, orthoMatrix.Begin());

		constexpr StringCrc lightColorSampler("s_lightingColor");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(lightColorSampler), screenTextureHandle);

		constexpr StringCrc bloomcolorSampler("s_bloom");
		bgfx::setTexture(1, GetRenderContext()->GetUniform(bloomcolorSampler), bgfx::getTexture(m_sampleChainFB[0]));

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		constexpr StringCrc CombineprogramName("CombineProgram");
		bgfx::submit(m_CombinePassID, GetRenderContext()->GetProgram(CombineprogramName));

		bgfx::blit(m_blitColorPassID, screenTextureHandle, 0, 0, bgfx::getTexture(m_combineFB));
	}

	void BloomRenderer::Blur(uint16_t width, uint16_t height, int iteration,float blursize, int blurscaling,cd::Matrix4x4 ortho,bgfx::TextureHandle texture)
	{
		width = static_cast<int>(width / blurscaling);
		height = static_cast<int>(height / blurscaling);

		const uint64_t tsFlags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
		for (int ii = 0; ii < 2; ++ii)
		{
			if (bgfx::isValid(m_blurChainFB[ii])) bgfx::destroy(m_blurChainFB[ii]);
			m_blurChainFB[ii] = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::RGBA32F, tsFlags);
		}

		uint16_t vertical = m_startVerticalBlurPassID;
		uint16_t horizontal = m_startHorizontalBlurPassID;
		for (int i = 0; i < iteration; i++)
		{
			float pixelSize[4] =
			{
				1.0f / static_cast<float>(width),
				1.0f / static_cast<float>(height),
				static_cast<float>(i / blurscaling) + blursize, /*i + blursize*/
				1.0f,
			};

			bgfx::setViewFrameBuffer(vertical, m_blurChainFB[0]);
			bgfx::setViewRect(vertical, 0, 0, width, height);
			bgfx::setViewTransform(vertical, nullptr, ortho.Begin());

			constexpr StringCrc textureSizeUniformName("u_textureSize");
			bgfx::setUniform(GetRenderContext()->GetUniform(textureSizeUniformName), pixelSize);

			constexpr StringCrc textureSampler("s_texture");
			bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), i == 0 ? texture : bgfx::getTexture(m_blurChainFB[1]));

			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
			Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

			constexpr StringCrc kawaseBlurprogramName("KawaseBlurProgram");
			bgfx::submit(vertical, GetRenderContext()->GetProgram(kawaseBlurprogramName));
			//constexpr StringCrc BlurHorizontalprogramName("BlurVerticalProgram"); // use Gaussian Blur
			//bgfx::submit(horizontal, GetRenderContext()->GetProgram(BlurHorizontalprogramName));

			// vertical
			bgfx::setViewFrameBuffer(horizontal, m_blurChainFB[1]);
			bgfx::setViewRect(horizontal, 0, 0, width, height);
			bgfx::setViewTransform(horizontal, nullptr, ortho.Begin());

			bgfx::setUniform(GetRenderContext()->GetUniform(textureSizeUniformName), pixelSize);

			bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), bgfx::getTexture(m_blurChainFB[0]));

			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
			Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

			bgfx::submit(horizontal, GetRenderContext()->GetProgram(kawaseBlurprogramName));
			//constexpr StringCrc BlurVerticalprogramName("BlurVerticalProgram");  // use Gaussian Blur
			//bgfx::submit(horizontal, GetRenderContext()->GetProgram(BlurVerticalprogramName));

			vertical += 2;
			horizontal += 2;
		}
	}
}
