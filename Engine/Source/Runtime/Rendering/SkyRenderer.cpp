#include "SkyRenderer.h"

#include "ECWorld/SceneWorld.h"
#include "RenderContext.h"

namespace engine
{

namespace
{

constexpr uint16_t sampleFlag = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr const char* skyboxSampler = "s_texSkybox";
constexpr const char *skyboxShader = "skyboxShader";

}

SkyRenderer::~SkyRenderer() = default;

void SkyRenderer::Init()
{
	m_pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	m_pRenderContext->CreateUniform(skyboxSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateTexture(m_pSkyComponent->GetRadianceTexturePath().c_str(), sampleFlag);

	m_pRenderContext->CreateProgram(skyboxShader, "vs_PBR_skybox.bin", "fs_PBR_skybox.bin");

	bgfx::setViewName(GetViewID(), "SkyRenderer");
}

void SkyRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	if (m_pSkyComponent->GetSkyType() != SkyType::SkyBox)
	{
		return;
	}

	bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());

	// We want the skybox to be centered around the player
	// so that no matter how far the player moves, the skybox won't get any closer.
	// Remove the translation part of the view matrix
	// so only rotation will affect the skybox's position vectors.
	float fixedViewMatrix[16];
	std::memcpy(fixedViewMatrix, pViewMatrix, 12 * sizeof(float));
	fixedViewMatrix[12] = fixedViewMatrix[13] = fixedViewMatrix[14] = 0.0f;
	fixedViewMatrix[15] = 1.0f;

	bgfx::setViewTransform(GetViewID(), fixedViewMatrix, pProjectionMatrix);
	bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
}

void SkyRenderer::Render(float deltaTime)
{
	if (m_pSkyComponent->GetSkyType() != SkyType::SkyBox)
	{
		return;
	}

	StaticMeshComponent *pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(m_pCurrentSceneWorld->GetSkyEntity());
	if (!pMeshComponent)
	{
		return;
	}
	bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{pMeshComponent->GetVertexBuffer()});
	bgfx::setIndexBuffer(bgfx::IndexBufferHandle{pMeshComponent->GetIndexBuffer()});

	constexpr StringCrc sampler(skyboxSampler);
	constexpr StringCrc program(skyboxShader);

	bgfx::setTexture(0,
		m_pRenderContext->GetUniform(sampler),
		m_pRenderContext->GetTexture(StringCrc(m_pSkyComponent->GetRadianceTexturePath())));

	bgfx::submit(GetViewID(), m_pRenderContext->GetProgram(program));
}

}
