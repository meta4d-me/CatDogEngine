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
	//	engine::ParticleEmitterComponent* pEmitterComponent = m_pCurrentSceneWorld->GetParticleEmitterComponent(entity);
	//	const cd::Transform& transform = m_pCurrentSceneWorld->GetTransformComponent(entity)->GetTransform();
	//	pEmitterComponent->init();
	//	int particleindex = pEmitterComponent->allocateParticleIndex();
	//	ParticleComponent& particle = pEmitterComponent->getParticle(particleindex);
	//	if (pEmitterComponent->allocateParticleIndex() != -1)
	//	{
	//			particle.setPos(transform.GetTranslation());
	//			particle.setVelocity(cd::Vec3f(-0.02f + rand() % 4100, 0.0f, 0.0f));
	//			particle.setAcceleration(cd::Vec3f(0.0f, 0.0f, 0.0f));
	//			particle.setColor(cd::Vec3f(1.0f, 0.0f, 0.0f));
	//			particle.setLifeTime(1.0f + rand() % 7);
	//	}	

	//	pEmitterComponent->update(1.0f / deltaTime);

	//	cd::VertexFormat	 vertexFormat; 
	//	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);
	//	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Color, cd::AttributeValueType::Float, 3);

	//	bgfx::VertexLayout vertexLayout;
	//	VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexLayout());

	//	//vertex
	//	pEmitterComponent->GetVertexBuffer().resize(2*sizeof(cd::Vec3f));


	//	std::memcpy(pEmitterComponent->GetVertexBuffer().data(), &particle.getPos(), sizeof(cd::Vec3f));
	//	std::memcpy(pEmitterComponent->GetVertexBuffer().data()+sizeof(cd::Vec3f), &particle.getColor(), sizeof(cd::Vec3f));


	//	//index
	///*	std::vector<uint16_t> indices = { 0, 1 };
	//	emitter->GetIndexBuffer().resize(2*sizeof(uint16_t));
	//	std::memcpy(emitter->GetIndexBuffer().data(), indices.data(), sizeof(indices));*/


	//	pEmitterComponent->setParticleVBH(bgfx::createVertexBuffer(bgfx::makeRef(pEmitterComponent->GetVertexBuffer().data(), static_cast<uint32_t>(pEmitterComponent->GetVertexBuffer().size())), vertexLayout).idx);
	//	//emitter->setParticleIBH( bgfx::createIndexBuffer(bgfx::makeRef(emitter->GetIndexBuffer().data(), static_cast<uint32_t>(emitter->GetIndexBuffer().size())),0U).idx);

	//	bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pEmitterComponent->getParticleVBH() });
	////	bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ emitter->getParticleIBH() });

	//	constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
	//		BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_POINTS;

	//	bgfx::setState(state);

	//	constexpr StringCrc ParticleProgram("ParticleProgram");
	//	bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(ParticleProgram));
	// 
	//const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();
}

}