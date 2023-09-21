#include "ParticleRenderer.h"

namespace engine {

void ParticleRenderer::Init()
{
	engine::StringCrc particleTexture("Textures/splash_texture.png");
	m_particleTextureHandle = GetRenderContext()->GetTexture(particleTexture);

	GetRenderContext()->CreateUniform("s_texColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateProgram("ParticleProgram", "vs_particle.bin", "fs_particle.bin");
	bgfx::setViewName(GetViewID(), "ParticleRenderer");
}

void ParticleRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void ParticleRenderer::Render(float deltaTime)
{
	for (Entity entity : m_pCurrentSceneWorld->GetParticleEmitterEntities())
	{
		engine::ParticleEmitterComponent* pEmitterComponent = m_pCurrentSceneWorld->GetParticleEmitterComponent(entity);
		const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(entity)->GetTransform();

		for (int i = 0; i < pEmitterComponent->GetParticleSystem().GetMaxCount(); ++i)
		{
			pEmitterComponent->GetParticleSystem().AllocateParticleIndex();
			pEmitterComponent->GetParticleSystem().SetPos(cameraTransform.GetTranslation());
		}

		pEmitterComponent->GetParticleSystem().UpdateActive(deltaTime);

		pEmitterComponent->UpdateBuffer();

		constexpr StringCrc ParticleSampler("s_texColor");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(ParticleSampler), m_particleTextureHandle);

		bgfx::setVertexBuffer(0, bgfx::DynamicVertexBufferHandle{ pEmitterComponent->GetParticleVBH() });
		bgfx::setIndexBuffer(bgfx::DynamicIndexBufferHandle{  pEmitterComponent->GetParticleIBH() });


		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_TRISTRIP;
		bgfx::setState(state);

		constexpr StringCrc ParticleProgram("ParticleProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(ParticleProgram));
	}
}

}