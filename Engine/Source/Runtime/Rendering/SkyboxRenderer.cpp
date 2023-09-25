#include "SkyboxRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkyComponent.h"
#include "Rendering/RenderContext.h"

namespace engine
{

namespace
{

constexpr const char* skyboxSampler = "s_texSkybox";
constexpr const char* skyboxProgram = "skyboxProgram";

constexpr uint16_t sampleFalg = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr uint64_t renderState = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LEQUAL;

}

void SkyboxRenderer::Init()
{
	constexpr StringCrc programCrc = StringCrc(skyboxProgram);
	GetRenderContext()->RegisterShaderProgram(programCrc, {"vs_skybox", "fs_skybox"});

	bgfx::setViewName(GetViewID(), "SkyboxRenderer");
}

void SkyboxRenderer::Warmup()
{
	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	GetRenderContext()->CreateUniform(skyboxSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateTexture(pSkyComponent->GetRadianceTexturePath().c_str(), sampleFalg);

	GetRenderContext()->UploadShaderProgram(skyboxProgram);
}

void SkyboxRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	if (!IsEnable())
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
	bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
}

void SkyboxRenderer::Render(float deltaTime)
{
	if (!IsEnable())
	{
		return;
	}

	StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(m_pCurrentSceneWorld->GetSkyEntity());
	if (!pMeshComponent)
	{
		return;
	}

	bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pMeshComponent->GetVertexBuffer() });
	bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pMeshComponent->GetIndexBuffer() });

	// Create a new TextureHandle each frame if the skybox texture path has been updated,
	// otherwise RenderContext::CreateTexture will automatically skip it.
	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());
	GetRenderContext()->CreateTexture(pSkyComponent->GetRadianceTexturePath().c_str(), sampleFalg);

	constexpr StringCrc samplerCrc(skyboxSampler);

	bgfx::setTexture(0,
		GetRenderContext()->GetUniform(samplerCrc),
		GetRenderContext()->GetTexture(StringCrc(pSkyComponent->GetRadianceTexturePath())));

	bgfx::setState(renderState);

	GetRenderContext()->Submit(GetViewID(), skyboxProgram);
}

bool SkyboxRenderer::IsEnable() const
{
	SkyType type = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity())->GetSkyType();

	// SkyboxRenderer handle both IBL and non-sky case.
	return SkyType::SkyBox == type || SkyType::None == type;
}

}