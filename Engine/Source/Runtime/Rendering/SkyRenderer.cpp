#include "SkyRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "RenderContext.h"

namespace engine
{

namespace
{

constexpr uint16_t sampleFalg = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr uint16_t renderState = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
constexpr const char* skyboxSampler = "s_texSkybox";
constexpr const char* skyboxShader = "skyboxShader";

}

SkyRenderer::~SkyRenderer() = default;

void SkyRenderer::Init()
{
	m_pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	GetRenderContext()->CreateUniform(skyboxSampler, bgfx::UniformType::Sampler);

	GetRenderContext()->CreateProgram(skyboxShader, "vs_PBR_skybox.bin", "fs_PBR_skybox.bin");

	bgfx::setViewName(GetViewID(), "SkyboxRenderer");
}

void SkyRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	if (m_pSkyComponent->GetSkyType() != SkyType::SkyBox)
	{
		return;
	}

	// We want the skybox to be centered around the player
	// so that no matter how far the player moves, the skybox won't get any closer.
	// Remove the translation part of the view matrix
	// so only rotation will affect the skybox's position vectors.
	float fixedViewMatrix[16];
	std::memcpy(fixedViewMatrix, pViewMatrix, 12 * sizeof(float));
	fixedViewMatrix[12] = fixedViewMatrix[13] = fixedViewMatrix[14] = 0.0f;
	fixedViewMatrix[15] = 1.0f;

	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), fixedViewMatrix, pProjectionMatrix);
	bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
}

void SkyRenderer::Render(float deltaTime)
{
	if (m_pSkyComponent->GetSkyType() != SkyType::SkyBox)
	{
		return;
	}

	StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(m_pCurrentSceneWorld->GetSkyEntity());
	if (!pMeshComponent)
	{
		return;
	}
	bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{pMeshComponent->GetVertexBuffer()});
	bgfx::setIndexBuffer(bgfx::IndexBufferHandle{pMeshComponent->GetIndexBuffer()});

	constexpr StringCrc sampler(skyboxSampler);
	constexpr StringCrc program(skyboxShader);

	// Create a new TextureHandle each frame if the skybox texture path has been updated,
	// otherwise RenderContext::CreateTexture will automatically skip.
	GetRenderContext()->CreateTexture(m_pSkyComponent->GetRadianceTexturePath().c_str(), sampleFalg);

	bgfx::setTexture(0,
		GetRenderContext()->GetUniform(sampler),
		GetRenderContext()->GetTexture(StringCrc(m_pSkyComponent->GetRadianceTexturePath())));

	bgfx::setState(renderState);
	bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(program));
}

}