#include "Log/Log.h"
#include "ParticleRenderer.h"
#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"

//#include <cstdlib>
namespace engine {
//float getRandomValue(float min, float max)
//{
//	return min + static_cast<float>(rand()) / (RAND_MAX / (max - min));
//}

namespace
{
constexpr const char* cameraPos = "u_cameraPos";
constexpr const char* particleUp = "u_particleUp";

uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_TRISTRIP;

constexpr const char* ParticleProgram = "ParticleProgram";
constexpr const char* WO_BillboardParticleProgram = "WO_BillboardParticleProgram";

constexpr StringCrc ParticleProgramCrc = StringCrc{ "ParticleProgram" };
constexpr StringCrc WO_BillboardParticleProgramCrc = StringCrc{ "WO_BillboardParticleProgram" };
}

void ParticleRenderer::Init()
{
	GetRenderContext()->RegisterShaderProgram(ParticleProgramCrc, { "vs_particle", "fs_particle" });
	GetRenderContext()->RegisterShaderProgram(WO_BillboardParticleProgramCrc, { "vs_wo_billboardparticle","fs_wo_billboardparticle" });

	bgfx::setViewName(GetViewID(), "ParticleRenderer");
}

void ParticleRenderer::Warmup()
{
	constexpr const char* particleTexture = "Textures/Particle.png";
	m_particleTextureHandle = GetRenderContext()->CreateTexture(particleTexture);
	GetRenderContext()->CreateUniform("s_texColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cameraPos, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(particleUp, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->UploadShaderProgram(ParticleProgram);
	GetRenderContext()->UploadShaderProgram(WO_BillboardParticleProgram);
}

void ParticleRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void ParticleRenderer::Render(float deltaTime)
{
	Entity pMainCameraEntity = m_pCurrentSceneWorld->GetMainCameraEntity();
	for (Entity entity : m_pCurrentSceneWorld->GetParticleEmitterEntities())
	{
		const cd::Transform& particleTransform = m_pCurrentSceneWorld->GetTransformComponent(entity)->GetTransform();
		const cd::Quaternion& particleRotation = m_pCurrentSceneWorld->GetTransformComponent(entity)->GetTransform().GetRotation();
		ParticleEmitterComponent* pEmitterComponent = m_pCurrentSceneWorld->GetParticleEmitterComponent(entity);
		
		const cd::Transform& pMainCameraTransform = m_pCurrentSceneWorld->GetTransformComponent(pMainCameraEntity)->GetTransform();
		//const cd::Quaternion& cameraRotation = pMainCameraTransform.GetRotation();
		//Not include particle attribute
		pEmitterComponent->GetParticlePool().SetParticleMaxCount(pEmitterComponent->GetSpawnCount());
		pEmitterComponent->GetParticlePool().AllParticlesReset();
		int particleIndex = pEmitterComponent->GetParticlePool().AllocateParticleIndex();
		if (particleIndex != -1)
		{
			Particle& particle = pEmitterComponent->GetParticlePool().GetParticle(particleIndex);
			//cd::Vec3f random(getRandomValue(-20.0f, 20.0f), getRandomValue(-20.0f, 20.0f), getRandomValue(-20.0f, 20.0f));
			particle.SetPos(particleTransform.GetTranslation());
			particle.SetSpeed(pEmitterComponent->GetEmitterVelocity());
			particle.SetAcceleration(pEmitterComponent->GetEmitterAcceleration());
			particle.SetColor(pEmitterComponent->GetEmitterColor());
			particle.SetLifeTime(pEmitterComponent->GetLifeTime());
		}

		pEmitterComponent->GetParticlePool().Update(1.0f/60.0f);

		//if (m_currentType != pEmitterComponent->GetEmitterParticleType())
		//{
		//	m_currentType = pEmitterComponent->GetEmitterParticleType();
		//	if (pEmitterComponent->GetMeshData() == nullptr)
		//	{
		//		pEmitterComponent->PaddingVertexBuffer();
		//		pEmitterComponent->PaddingIndexBuffer();
		//	}
		//	else
		//	{
		//		pEmitterComponent->ParseMeshVertexBuffer();
		//		pEmitterComponent->ParseMeshIndexBuffer();
		//	}
		//	const bgfx::Memory *pParticleVertexBuffer = bgfx::makeRef(pEmitterComponent->GetVertexBuffer().data(), static_cast<uint32_t>(pEmitterComponent->GetVertexBuffer().size()));
		//	const bgfx::Memory *pParticleIndexBuffer = bgfx::makeRef(pEmitterComponent->GetIndexBuffer().data(), static_cast<uint32_t>(pEmitterComponent->GetIndexBuffer().size()));
		//	bgfx::update(bgfx::DynamicVertexBufferHandle{ pEmitterComponent->GetParticleVertexBufferHandle()}, 0, pParticleVertexBuffer);
		//	bgfx::update(bgfx::DynamicIndexBufferHandle{pEmitterComponent->GetParticleIndexBufferHandle()}, 0, pParticleIndexBuffer);
		//}

		//Particle Emitter Instance
		const uint16_t instanceStride = 80;
		// to total number of instances to draw
		uint32_t totalSprites;
		totalSprites = pEmitterComponent->GetParticlePool().GetParticleMaxCount();
		uint32_t drawnSprites = bgfx::getAvailInstanceDataBuffer(totalSprites, instanceStride);

		bgfx::InstanceDataBuffer idb;
		bgfx::allocInstanceDataBuffer(&idb, drawnSprites, instanceStride);

		uint8_t* data = idb.data;
		for (uint32_t ii = 0; ii < drawnSprites; ++ii)
		{
			float* mtx = (float*)data;
			bx::mtxSRT(mtx, particleTransform.GetScale().x(), particleTransform.GetScale().y(), particleTransform.GetScale().z(),
				particleRotation.Pitch(), particleRotation.Yaw(), particleRotation.Roll(),
				pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().x(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().y(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().z());

				//auto up = particleTransform.GetRotation().ToMatrix3x3() * cd::Vec3f(0, 1, 0);
				//auto vec =  pMainCameraTransform.GetTranslation() - pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos();
				//auto right = up.Cross(vec);
				//float yaw = atan2f(right.z(), right.x());
				//float pitch = atan2f(vec.y(), sqrtf(vec.x() * vec.x() + vec.z() * vec.z())); 
				//float roll = atan2f(right.x(), -right.y()); 
				//bx::mtxSRT(mtx, particleTransform.GetScale().x(), particleTransform.GetScale().y(), particleTransform.GetScale().z(),
				//	pitch, yaw, roll,
				//	pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().x(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().y(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().z());

			float* color = (float*)&data[64];
			color[0] = pEmitterComponent->GetEmitterColor().x();
			color[1] = pEmitterComponent->GetEmitterColor().y();
			color[2] = pEmitterComponent->GetEmitterColor().z();
			color[3] = pEmitterComponent->GetEmitterColor().w();

			data += instanceStride;
		}
		constexpr StringCrc cameraPosCrc(cameraPos);
		bgfx::setUniform(GetRenderContext()->GetUniform(cameraPosCrc), &pMainCameraTransform.GetTranslation(), 1);

		constexpr StringCrc particleUpCrc(particleUp);
		auto up = particleTransform.GetRotation().ToMatrix3x3() * cd::Vec3f(0, 1, 0);
		bgfx::setUniform(GetRenderContext()->GetUniform(particleUpCrc), &up, 1);

		constexpr StringCrc ParticleSampler("s_texColor");
		bgfx::setTexture(0, GetRenderContext()->GetUniform(ParticleSampler), m_particleTextureHandle);
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pEmitterComponent->GetParticleVertexBufferHandle() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{  pEmitterComponent->GetParticleIndexBufferHandle() });


		bgfx::setInstanceDataBuffer(&idb);

		bgfx::setState(state);

		if (pEmitterComponent->GetRenderMode() == engine::ParticleRenderMode::Mesh)
		{
			GetRenderContext()->Submit(GetViewID(), ParticleProgram);
		}
		else if (pEmitterComponent->GetRenderMode() == engine::ParticleRenderMode::Billboard)
		{
			GetRenderContext()->Submit(GetViewID(), WO_BillboardParticleProgram);
		}
	}
}

}