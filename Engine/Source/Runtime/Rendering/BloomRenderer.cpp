#include "BloomRenderer.h"

#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"

#include <format>

namespace engine
{

namespace
{

constexpr const char* CapTureBrightnessProgram = "CapTureBrightnessProgram";
constexpr const char* DownSampleProgram = "DownSampleProgram";
constexpr const char* BlurVerticalProgram = "BlurVerticalProgram";
constexpr const char* BlurHorizontalProgram = "BlurHorizontalProgram";
constexpr const char* UpSampleProgram = "UpSampleProgram";
constexpr const char* KawaseBlurProgram = "KawaseBlurProgram";
constexpr const char* CombineProgram = "CombineProgram";

constexpr StringCrc CapTureBrightnessProgramCrc{ CapTureBrightnessProgram };
constexpr StringCrc DownSampleProgramCrc{ DownSampleProgram };
constexpr StringCrc BlurVerticalProgramCrc{ BlurVerticalProgram };
constexpr StringCrc BlurHorizontalProgramCrc{ BlurHorizontalProgram };
constexpr StringCrc UpSampleProgramCrc{ UpSampleProgram };
constexpr StringCrc KawaseBlurProgramCrc{ KawaseBlurProgram };
constexpr StringCrc CombineProgramCrc{ CombineProgram };

}

void BloomRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(CapTureBrightnessProgram, "vs_fullscreen", "fs_captureBrightness"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(DownSampleProgram, "vs_fullscreen", "fs_dowmsample"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(BlurVerticalProgram, "vs_fullscreen", "fs_blurvertical"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(BlurHorizontalProgram, "vs_fullscreen", "fs_blurhorizontal"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(UpSampleProgram, "vs_fullscreen", "fs_upsample"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(KawaseBlurProgram, "vs_fullscreen", "fs_kawaseblur"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(CombineProgram, "vs_fullscreen", "fs_bloom"));

	GetRenderContext()->CreateUniform("s_texture", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("s_bloom", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("s_lightingColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("u_textureSize", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("u_bloomIntensity", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateUniform("u_luminanceThreshold", bgfx::UniformType::Vec4);

	bgfx::setViewName(GetViewID(), "BloomRenderer");

	AllocateViewIDs();
}

void BloomRenderer::AllocateViewIDs()
{
	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
	assert(pCameraComponent);

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

	int totalBlurTimes = pCameraComponent->GetBlurMaxTimes() * 2;
	for (int i = 0; i < totalBlurTimes - 2; i++)
	{
		GetRenderContext()->CreateView();
	}

	m_startUpSamplePassID = GetRenderContext()->CreateView();
	for (int i = 0; i < TEX_CHAIN_LEN - 2; i++)
	{
		GetRenderContext()->CreateView();
	}
	m_combinePassID = GetRenderContext()->CreateView();
	m_blitColorPassID = GetRenderContext()->CreateView();
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
		tempW = GetRenderContext()->GetBackBufferWidth();
		tempH = GetRenderContext()->GetBackBufferHeight();
	}

	if (m_width != tempW || m_height != tempH)
	{
		m_width = tempW;
		m_height = tempH;

		Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
		CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);

		constexpr uint64_t tsFlags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
		for (int ii = 0; ii < TEX_CHAIN_LEN; ++ii)
		{
			if (bgfx::isValid(m_sampleChainFB[ii]))
			{
				bgfx::destroy(m_sampleChainFB[ii]);
			}

			int viewWidth = m_width >> ii;
			int viewHeight = m_height >> ii;
			if (viewWidth < 2 || viewHeight < 2)
			{
				pCameraComponent->SetBloomDownSampleMaxTimes(std::max(ii - 1, 0));
				break;
			}

			m_sampleChainFB[ii] = bgfx::createFrameBuffer(viewWidth, viewHeight, bgfx::TextureFormat::RGBA32F, tsFlags);
		}

		m_combineFB = bgfx::createFrameBuffer(m_width, m_height, bgfx::TextureFormat::RGBA32F, tsFlags);
	}
}

void BloomRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

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
	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.begin());

	constexpr StringCrc luminanceThresholdUniformName("u_luminanceThreshold");
	bgfx::setUniform(GetRenderContext()->GetUniform(luminanceThresholdUniformName), &pCameraComponent->GetLuminanceThreshold());

	constexpr StringCrc textureSampler("s_texture");
	bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), screenEmissColorTextureHandle);

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

	GetRenderContext()->Submit(GetViewID(), CapTureBrightnessProgramCrc);

	// downsample
	int sampleTimes = std::min(pCameraComponent->GetBloomDownSampleTimes(), pCameraComponent->GetBloomDownSampleMaxTimes());
	int tempshift = 0;
	for (int sampleIndex = 0; sampleIndex < sampleTimes; ++sampleIndex)
	{
		int shift = sampleIndex + 1;
		tempshift = shift;
		const float pixelSize[4] =
		{
			1.0f / static_cast<float>(m_width >> shift),
			1.0f / static_cast<float>(m_height >> shift),
			0.0f,
			0.0f,
		};

		bgfx::setViewFrameBuffer(m_startDowmSamplePassID + sampleIndex, m_sampleChainFB[shift]);
		bgfx::setViewName(m_startDowmSamplePassID + sampleIndex, std::format("Downsample_{}", sampleIndex).c_str());
		bgfx::setViewRect(m_startDowmSamplePassID + sampleIndex, 0, 0, m_width >> shift, m_height >> shift);
		bgfx::setViewTransform(m_startDowmSamplePassID + sampleIndex, nullptr, orthoMatrix.begin());

		constexpr StringCrc textureSizeUniformName("u_textureSize");
		bgfx::setUniform(GetRenderContext()->GetUniform(textureSizeUniformName), pixelSize);

		bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), bgfx::getTexture(m_sampleChainFB[sampleIndex]));

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		GetRenderContext()->Submit(m_startDowmSamplePassID + sampleIndex, DownSampleProgramCrc);
	}

	if (pCameraComponent->GetIsBlurEnable() && pCameraComponent->GetBlurTimes() != 0)
	{
		Blur(m_width >> tempshift, m_height >> tempshift, pCameraComponent->GetBlurTimes(), pCameraComponent->GetBlurSize(), pCameraComponent->GetBlurScaling(), orthoMatrix, bgfx::getTexture(m_sampleChainFB[tempshift]));
	}

	// upsample
	for (int sampleIndex = 0; sampleIndex < sampleTimes; ++sampleIndex)
	{
		int shift = sampleTimes - sampleIndex - 1;
		const float pixelSize[4] =
		{
			1.0f / static_cast<float>(m_width >> shift),
			1.0f / static_cast<float>(m_height >> shift),
			0.0f,
			0.0f,
		};

		bgfx::setViewFrameBuffer(m_startUpSamplePassID + sampleIndex, m_sampleChainFB[shift]);
		bgfx::setViewName(m_startUpSamplePassID + sampleIndex, std::format("Upsample_{}", sampleIndex).c_str());
		bgfx::setViewRect(m_startUpSamplePassID + sampleIndex, 0, 0, m_width >> shift, m_height >> shift);
		bgfx::setViewTransform(m_startUpSamplePassID + sampleIndex, nullptr, orthoMatrix.begin());

		constexpr StringCrc textureSizeUniformName("u_textureSize");
		bgfx::setUniform(GetRenderContext()->GetUniform(textureSizeUniformName), pixelSize);

		constexpr StringCrc bloomIntensityUniformName("u_bloomIntensity");
		bgfx::setUniform(GetRenderContext()->GetUniform(bloomIntensityUniformName), &pCameraComponent->GetBloomIntensity());

		if (pCameraComponent->GetIsBlurEnable() && pCameraComponent->GetBlurTimes() != 0 && 0 == sampleIndex)
		{
			bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), bgfx::getTexture(m_blurChainFB[1]));
		}
		else
		{
			bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), bgfx::getTexture(m_sampleChainFB[shift + 1]));
		}

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ADD);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		GetRenderContext()->Submit(m_startUpSamplePassID + sampleIndex, UpSampleProgramCrc);
	}

	// combine 
	bgfx::setViewFrameBuffer(m_combinePassID, m_combineFB);
	bgfx::setViewName(m_combinePassID, "CombineBloom");
	bgfx::setViewRect(m_combinePassID, 0, 0, m_width, m_height);
	bgfx::setViewTransform(m_combinePassID, nullptr, orthoMatrix.begin());

	constexpr StringCrc lightColorSampler("s_lightingColor");
	bgfx::setTexture(0, GetRenderContext()->GetUniform(lightColorSampler), screenTextureHandle);

	constexpr StringCrc bloomcolorSampler("s_bloom");
	bgfx::setTexture(1, GetRenderContext()->GetUniform(bloomcolorSampler), bgfx::getTexture(m_sampleChainFB[0]));

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

	GetRenderContext()->Submit(m_combinePassID, CombineProgramCrc);

	bgfx::blit(m_blitColorPassID, screenTextureHandle, 0, 0, bgfx::getTexture(m_combineFB));
}

void BloomRenderer::Blur(uint16_t width, uint16_t height, int iteration, float blursize, int blurscaling, cd::Matrix4x4 ortho, bgfx::TextureHandle texture)
{
	width = static_cast<int>(width / blurscaling);
	height = static_cast<int>(height / blurscaling);

	const uint64_t tsFlags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	for (int ii = 0; ii < 2; ++ii)
	{
		if (bgfx::isValid(m_blurChainFB[ii])) bgfx::destroy(m_blurChainFB[ii]);
		m_blurChainFB[ii] = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::RGBA32F, tsFlags);
	}

	uint16_t verticalViewID = m_startVerticalBlurPassID;
	uint16_t horizontalViewID = m_startHorizontalBlurPassID;
	for (int i = 0; i < iteration; i++)
	{
		float pixelSize[4] =
		{
			1.0f / static_cast<float>(width),
			1.0f / static_cast<float>(height),
			static_cast<float>(i / blurscaling) + blursize, /*i + blursize*/
			1.0f,
		};

		bgfx::setViewFrameBuffer(verticalViewID, m_blurChainFB[0]);
		bgfx::setViewName(verticalViewID, "BlurVertical");
		bgfx::setViewRect(verticalViewID, 0, 0, width, height);
		bgfx::setViewTransform(verticalViewID, nullptr, ortho.begin());

		constexpr StringCrc textureSizeUniformName("u_textureSize");
		bgfx::setUniform(GetRenderContext()->GetUniform(textureSizeUniformName), pixelSize);

		constexpr StringCrc textureSampler("s_texture");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), i == 0 ? texture : bgfx::getTexture(m_blurChainFB[1]));

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		GetRenderContext()->Submit(verticalViewID, KawaseBlurProgramCrc);

		//constexpr StringCrc BlurHorizontalprogramName("BlurVerticalProgram"); // use Gaussian Blur
		//bgfx::submit(horizontal, GetRenderContext()->GetProgram(BlurHorizontalprogramName));

		// vertical
		bgfx::setViewFrameBuffer(horizontalViewID, m_blurChainFB[1]);
		bgfx::setViewName(horizontalViewID, "BlurHorizontal");
		bgfx::setViewRect(horizontalViewID, 0, 0, width, height);
		bgfx::setViewTransform(horizontalViewID, nullptr, ortho.begin());

		bgfx::setUniform(GetRenderContext()->GetUniform(textureSizeUniformName), pixelSize);

		bgfx::setTexture(0, GetRenderContext()->GetUniform(textureSampler), bgfx::getTexture(m_blurChainFB[0]));

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

		GetRenderContext()->Submit(horizontalViewID, KawaseBlurProgramCrc);

		//constexpr StringCrc BlurVerticalprogramName("BlurVerticalProgram");  // use Gaussian Blur
		//bgfx::submit(horizontal, GetRenderContext()->GetProgram(BlurVerticalprogramName));

		verticalViewID += 2;
		horizontalViewID += 2;
	}
}

}
