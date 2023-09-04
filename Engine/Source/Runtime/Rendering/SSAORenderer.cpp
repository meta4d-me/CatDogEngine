#include "SSAORenderer.h"

#include "RenderContext.h"
#include <random>
#include <bx/math.h>

namespace engine
{
void SSAORenderer::Init()
{
	GetRenderContext()->CreateUniform("s_noiseTexture", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("s_positionTexture", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("s_normalTexture", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("u_samples", bgfx::UniformType::Vec4, 64);
	GetRenderContext()->CreateUniform("u_screenSize", bgfx::UniformType::Vec4);
	GetRenderContext()->CreateProgram("SSAOProgram", "vs_fullscreen.bin", "fs_ssao.bin");

	bgfx::setViewName(GetViewID(), "SSAORenderer");

	m_ssaoFB = BGFX_INVALID_HANDLE;
	m_noiseTexture = BGFX_INVALID_HANDLE;

	GenerateNoiseTexture();
}

SSAORenderer::~SSAORenderer()
{
}

void SSAORenderer::SetEnable(bool value)
{
	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
	pCameraComponent->SetSSAOEnable(value);
}

bool SSAORenderer::IsEnable() const
{
	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);
	return pCameraComponent->GetIsSSAOEnable();
}

void SSAORenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
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
		m_ssaoFB = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::RGBA32F, tsFlags);
	}
}

void SSAORenderer::Render(float deltaTime)
{
	constexpr StringCrc sceneRenderTarget("SceneRenderTarget");

	const RenderTarget* pInputRT = GetRenderContext()->GetRenderTarget(sceneRenderTarget);
	const RenderTarget* pOutputRT = GetRenderTarget();

	bgfx::TextureHandle screenViewPositionTextureHandle;
	bgfx::TextureHandle screenViewNormalTextureHandle;
	bgfx::TextureHandle screenTextureHandle;
	if (pInputRT == pOutputRT)
	{
		constexpr StringCrc sceneRenderTargetBlitViewPosition("SceneRenderTargetBlitViewPosition");
		screenViewPositionTextureHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitViewPosition);

		constexpr StringCrc sceneRenderTargetBlitViewNormal("SceneRenderTargetBlitViewNormal");
		screenViewNormalTextureHandle = GetRenderContext()->GetTexture(sceneRenderTargetBlitViewNormal);
	}
	else
	{
		screenTextureHandle = pInputRT->GetTextureHandle(0);
		screenViewPositionTextureHandle = pInputRT->GetTextureHandle(2);
		screenViewNormalTextureHandle = pInputRT->GetTextureHandle(3);
	}

	Entity entity = m_pCurrentSceneWorld->GetMainCameraEntity();
	CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(entity);

	cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

	bgfx::setViewFrameBuffer(GetViewID(), m_ssaoFB);
	bgfx::setViewRect(GetViewID(), 0, 0, width, height);
	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.Begin());

	constexpr StringCrc noiseTextureSampler("s_noiseTexture");
	bgfx::setTexture(0, GetRenderContext()->GetUniform(noiseTextureSampler), m_noiseTexture);

	constexpr StringCrc positionTextureSampler("s_positionTexture");
	bgfx::setTexture(1, GetRenderContext()->GetUniform(positionTextureSampler), screenViewPositionTextureHandle);

	constexpr StringCrc normalTextureSampler("s_normalTexture");
	bgfx::setTexture(2, GetRenderContext()->GetUniform(normalTextureSampler), screenViewNormalTextureHandle);

	constexpr StringCrc samplesUniformName("u_samples");
	bgfx::setUniform(GetRenderContext()->GetUniform(samplesUniformName), &ssaoKernel[0], static_cast <uint16_t>(64));

	float screenSize[4] = { static_cast<float>(width),static_cast<float>(height),0.0f,0.0f };
	constexpr StringCrc screenSizeUniformName("u_screenSize");
	bgfx::setUniform(GetRenderContext()->GetUniform(screenSizeUniformName), screenSize);

	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	Renderer::ScreenSpaceQuad(GetRenderTarget(), false);

	constexpr StringCrc ssaoprogramName("SSAOProgram");
	bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(ssaoprogramName));
}

void SSAORenderer::GenerateNoiseTexture()
{
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine generator;
	for (unsigned int i = 0; i < 64; ++i)
	{
		bx::Vec3 sample(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator));
		sample = bx::normalize(sample);
		sample.x *= randomFloats(generator);
		sample.y *= randomFloats(generator);
		sample.z *= randomFloats(generator);
		float scale = float(i) / 64.0f;

		scale = bx::lerp(0.1f, 1.0f, scale * scale);
		sample.x *= scale;
		sample.y *= scale;
		sample.z *= scale;

		ssaoKernel[i * 4 + 0] = sample.x;
		ssaoKernel[i * 4 + 1] = sample.y;
		ssaoKernel[i * 4 + 2] = sample.z;
		ssaoKernel[i * 4 + 3] = 0.0f;
	}

	std::vector<float> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		bx::Vec3 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, 0.0f);
		ssaoNoise.push_back(noise.x);
		ssaoNoise.push_back(noise.y);
		ssaoNoise.push_back(noise.z);
		ssaoNoise.push_back(0.0f);
	}
	const bgfx::Memory* mem = bgfx::copy(ssaoNoise.data(), uint32_t(ssaoNoise.size()) * sizeof(float));
	m_noiseTexture = bgfx::createTexture2D(4, 4, false, 1, bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_RT | BGFX_SAMPLER_U_MIRROR | BGFX_SAMPLER_V_MIRROR, mem);
}
}