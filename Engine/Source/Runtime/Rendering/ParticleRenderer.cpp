#include "Log/Log.h"
#include "ParticleRenderer.h"
#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"

namespace engine {

void ParticleRenderer::Init()
{
	constexpr StringCrc ParticleProgram = StringCrc{ "ParticleProgram" };
	GetRenderContext()->RegisterShaderProgram(ParticleProgram, { "vs_particle", "fs_particle" });
	bgfx::setViewName(GetViewID(), "ParticleRenderer");
}

void ParticleRenderer::Warmup()
{
	constexpr const char* particleTexture = "Textures/Particle.png";
	m_particleTextureHandle = GetRenderContext()->CreateTexture(particleTexture);
	GetRenderContext()->CreateUniform("s_texColor", bgfx::UniformType::Sampler);
	GetRenderContext()->UploadShaderProgram("ParticleProgram");
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
		const cd::Transform& particleTransform = m_pCurrentSceneWorld->GetTransformComponent(entity)->GetTransform();
		const cd::Quaternion& particleRotation = m_pCurrentSceneWorld->GetTransformComponent(entity)->GetTransform().GetRotation();
		ParticleEmitterComponent* pEmitterComponent = m_pCurrentSceneWorld->GetParticleEmitterComponent(entity);

		pEmitterComponent->GetParticlePool().AllParticlesReset();
		int particleIndex = pEmitterComponent->GetParticlePool().AllocateParticleIndex();
		if (particleIndex != -1)
		{
			Particle& particle = pEmitterComponent->GetParticlePool().GetParticle(particleIndex);
			particle.SetPos(particleTransform.GetTranslation());
			particle.SetSpeed(pEmitterComponent->GetEmitterVelocity());
			particle.SetAcceleration(pEmitterComponent->GetEmitterAcceleration());
			particle.SetColor(pEmitterComponent->GetEmitterColor());
			particle.SetLifeTime(pEmitterComponent->GetLifeTime());
		}

		pEmitterComponent->GetParticlePool().Update(1.0f/60.0f);

		//Particle Data Submit
		pEmitterComponent->PaddingVertexBuffer();
		pEmitterComponent->PaddingIndexBuffer();

		//Particle Emitter Insta
		const uint16_t instanceStride = 80;
		// to total number of instances to draw
		uint32_t totalSprites = pEmitterComponent->GetParticlePool().GetParticleMaxCount();
		uint32_t drawnSprites = bgfx::getAvailInstanceDataBuffer(totalSprites, instanceStride);

		bgfx::InstanceDataBuffer idb;
		bgfx::allocInstanceDataBuffer(&idb, drawnSprites, instanceStride);

		uint8_t* data = idb.data;

		for (uint32_t ii = 0; ii < drawnSprites; ++ii)
		{
			float* mtx = (float*)data;
			//bx::mtxRotateXY(mtx,0.0f, 0.0f);
			bx::mtxRotateXYZ(mtx, particleRotation.Pitch(), particleRotation.Yaw(), particleRotation.Roll());
			//bx::mtxRotateXYZ(mtx, 1.0f, 1.0f, 1.0f);
			mtx[12] = pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().x();
			mtx[13] = pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().y();
			mtx[14] = pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().z();
			float* color = (float*)&data[64];
			color[0] = pEmitterComponent->GetEmitterColor().x();
			color[1] = pEmitterComponent->GetEmitterColor().y();
			color[2] = pEmitterComponent->GetEmitterColor().z();
			color[3] = pEmitterComponent->GetEmitterColor().w();

			data += instanceStride;
		}

		constexpr StringCrc ParticleSampler("s_texColor");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(ParticleSampler), m_particleTextureHandle);
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pEmitterComponent->GetParticleVertexBufferHandle() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{  pEmitterComponent->GetParticleIndexBufferHandle() });

		bgfx::setInstanceDataBuffer(&idb);

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_TRISTRIP;
		bgfx::setState(state);

		GetRenderContext()->Submit(GetViewID(), "ParticleProgram");
	}
}

}