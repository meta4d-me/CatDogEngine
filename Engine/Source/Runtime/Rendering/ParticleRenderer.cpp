#include "ParticleRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/ParticleForceFieldComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Log/Log.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"
#include "../UniformDefines/U_Particle.sh"

namespace engine
{

namespace
{

constexpr const char* particlePos = "u_particlePos";
constexpr const char* particleScale = "u_particleScale";
constexpr const char* shapeRange = "u_shapeRange";
constexpr const char* particleColor = "u_particleColor";
constexpr const char* ribbonCount = "u_ribbonCount";
constexpr const char* ribbonMaxPos = "u_ribbonMaxPos";

uint64_t state_tristrip = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_TRISTRIP;

uint64_t state_lines = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;

//Sprite Particle in EditorApp.cpp
constexpr const char* RibbonParticleCsProgram = "RibbonParticleCsProgram";
constexpr const char* RibbonParticleProgram = "RibbonParticleProgram";
constexpr const char* ParticleEmitterShapeProgram = "ParticleEmitterShapeProgram";
constexpr const char* WO_BillboardParticleProgram = "WO_BillboardParticleProgram";

constexpr StringCrc RibbonParticleProgramCsCrc = StringCrc{ RibbonParticleCsProgram };
constexpr StringCrc RibbonParticleProgramCrc = StringCrc{ RibbonParticleProgram };

constexpr StringCrc ParticleEmitterShapeProgramCrc = StringCrc{ ParticleEmitterShapeProgram };
constexpr StringCrc WO_BillboardParticleProgramCrc = StringCrc{ WO_BillboardParticleProgram };
}

void ParticleRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(RibbonParticleCsProgram, "cs_particleRibbon", ShaderProgramType::Compute));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(RibbonParticleProgram, "vs_particleRibbon", "fs_particleRibbon"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(ParticleEmitterShapeProgram, "vs_particleEmitterShape", "fs_particleEmitterShape"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram(WO_BillboardParticleProgram, "vs_wo_billboardparticle", "fs_wo_billboardparticle"));

	constexpr const char* particleTexture = "Textures/textures/Particle.png";
	constexpr const char* ribbonTexture = "Textures/textures/Particle.png";
	m_particleSpriteTextureHandle = GetRenderContext()->CreateTexture(particleTexture);
	m_particleRibbonTextureHandle = GetRenderContext()->CreateTexture(ribbonTexture);

	GetRenderContext()->CreateUniform("s_texColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("r_texColor", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(particlePos, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(particleScale, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(shapeRange, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(particleColor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(ribbonCount, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(ribbonMaxPos, bgfx::UniformType::Vec4, 300);

	bgfx::setViewName(GetViewID(), "ParticleRenderer");
}

void ParticleRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void ParticleRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	for (Entity entity : m_pCurrentSceneWorld->GetParticleForceFieldEntities())
	{
		ParticleForceFieldComponent* pForceFieldComponent = m_pCurrentSceneWorld->GetParticleForceFieldComponent(entity);
		const cd::Transform& forcefieldTransform = m_pCurrentSceneWorld->GetTransformComponent(entity)->GetTransform();
		SetForceFieldRotationForce(pForceFieldComponent);
		SetForceFieldRange(pForceFieldComponent,  forcefieldTransform.GetScale());
	}

	Entity pMainCameraEntity = m_pCurrentSceneWorld->GetMainCameraEntity();
	for (Entity entity : m_pCurrentSceneWorld->GetParticleEmitterEntities())
	{
		const cd::Transform& particleTransform = m_pCurrentSceneWorld->GetTransformComponent(entity)->GetTransform();
		const cd::Quaternion& particleRotation = m_pCurrentSceneWorld->GetTransformComponent(entity)->GetTransform().GetRotation();
		ParticleEmitterComponent* pEmitterComponent = m_pCurrentSceneWorld->GetParticleEmitterComponent(entity);
		ParticleRibbonComponent* pRibbonEmitterComponet = m_pCurrentSceneWorld->GetParticleRibbonComponent(entity);
		MaterialComponent* pParticleMaterialComponet = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		//NOTE: This ShaderResource Not Used Just For Judge
		const ShaderResource* pShaderResource = pParticleMaterialComponet->GetShaderResource();
		if (ResourceStatus::Ready != pShaderResource->GetStatus() &&
			ResourceStatus::Optimized != pShaderResource->GetStatus())
		{
			continue;
		}

		const cd::Transform& pMainCameraTransform = m_pCurrentSceneWorld->GetTransformComponent(pMainCameraEntity)->GetTransform();
		//const cd::Quaternion& cameraRotation = pMainCameraTransform.GetRotation();
		//Not include particle attribute
		pEmitterComponent->GetParticlePool().SetParticleMaxCount(pEmitterComponent->GetSpawnCount());
		pEmitterComponent->GetParticlePool().AllParticlesReset();
		int particleIndex = pEmitterComponent->GetParticlePool().AllocateParticleIndex();

		//Random value
		cd::Vec3f randomPos(getRandomValue(-pEmitterComponent->GetEmitterShapeRange().x(), pEmitterComponent->GetEmitterShapeRange().x()),
									getRandomValue(-pEmitterComponent->GetEmitterShapeRange().y(), pEmitterComponent->GetEmitterShapeRange().y()),
									getRandomValue(-pEmitterComponent->GetEmitterShapeRange().z(), pEmitterComponent->GetEmitterShapeRange().z()));	
		cd::Vec3f randomVelocity(getRandomValue(-pEmitterComponent->GetRandomVelocity().x(), pEmitterComponent->GetRandomVelocity().x()),
									getRandomValue(-pEmitterComponent->GetRandomVelocity().y(), pEmitterComponent->GetRandomVelocity().y()),
									getRandomValue(-pEmitterComponent->GetRandomVelocity().z(), pEmitterComponent->GetRandomVelocity().z()));
		pEmitterComponent->SetRandomPos(randomPos);

		//particle
		if (particleIndex != -1)
		{
			Particle& particle = pEmitterComponent->GetParticlePool().GetParticle(particleIndex);
			SetRandomPosState(particle, particleTransform.GetTranslation(), randomPos, pEmitterComponent->GetRandomPosState());
			SetRandomVelocityState(particle, pEmitterComponent->GetEmitterVelocity(), randomVelocity, pEmitterComponent->GetRandomVelocityState());
			particle.SetRotationForceField(m_forcefieldRotationFoce);
			particle.SetRotationForceFieldRange(m_forcefieldRange);
			particle.SetAcceleration(pEmitterComponent->GetEmitterAcceleration());
			particle.SetColor(pEmitterComponent->GetEmitterColor());
			particle.SetLifeTime(pEmitterComponent->GetLifeTime());
		}

		pEmitterComponent->GetParticlePool().Update(1.0f/60.0f);

		if (pEmitterComponent->GetInstanceState())
		{
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
				
				float* color = (float*)&data[64];
				color[0] = pEmitterComponent->GetEmitterColor().x();
				color[1] = pEmitterComponent->GetEmitterColor().y();
				color[2] = pEmitterComponent->GetEmitterColor().z();
				color[3] = pEmitterComponent->GetEmitterColor().w();

				data += instanceStride;
			}

			//Billboard particlePos particleScale
			constexpr StringCrc particlePosCrc(particlePos);
			bgfx::setUniform(GetRenderContext()->GetUniform(particlePosCrc), &particleTransform.GetTranslation(), 1);
			constexpr StringCrc ParticleScaleCrc(particleScale);
			bgfx::setUniform(GetRenderContext()->GetUniform(ParticleScaleCrc), &particleTransform.GetScale(), 1);

			if (pEmitterComponent->GetEmitterParticleType() == engine::ParticleType::Sprite)
			{
				constexpr StringCrc ParticleSampler("s_texColor");
				bgfx::setTexture(0, GetRenderContext()->GetUniform(ParticleSampler), m_particleSpriteTextureHandle);
				bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pEmitterComponent->GetSpriteParticleVertexBufferHandle() });
				bgfx::setIndexBuffer(bgfx::IndexBufferHandle{  pEmitterComponent->GetSpriteParticleIndexBufferHandle() });
			}
			else if (pEmitterComponent->GetEmitterParticleType() == engine::ParticleType::Ribbon)
			{
				constexpr StringCrc ribbonParticleSampler("r_texColor");
				bgfx::setTexture(1, GetRenderContext()->GetUniform(ribbonParticleSampler), m_particleRibbonTextureHandle);
				bgfx::setVertexBuffer(0, bgfx::DynamicVertexBufferHandle{ pRibbonEmitterComponet->GetRibbonParticlePrePosVertexBufferHandle() });
				bgfx::setVertexBuffer(1, bgfx::VertexBufferHandle{ pRibbonEmitterComponet->GetRibbonParticleRemainVertexBufferHandle() });
				bgfx::setIndexBuffer(bgfx::IndexBufferHandle{  pRibbonEmitterComponet->GetRibbonParticleIndexBufferHandle() });
			}

			bgfx::setState(state_tristrip);
			bgfx::setInstanceDataBuffer(&idb);

			SetRenderMode(pEmitterComponent->GetRenderMode(), pEmitterComponent->GetEmitterParticleType(), pParticleMaterialComponet);
		}
		else
		{
			constexpr StringCrc particleColorCrc(particleColor);
			bgfx::setUniform(GetRenderContext()->GetUniform(particleColorCrc), &pEmitterComponent->GetEmitterColor(), 1);

			uint32_t drawnSprites = pEmitterComponent->GetParticlePool().GetParticleMaxCount();
			for (uint32_t ii = 0; ii < drawnSprites; ++ii)
			{
				float mtx[16];
				if (pEmitterComponent->GetRenderMode() == engine::ParticleRenderMode::Mesh)
				{
					bx::mtxSRT(mtx, particleTransform.GetScale().x(), particleTransform.GetScale().y(), particleTransform.GetScale().z(),
						particleRotation.Pitch(), particleRotation.Yaw(), particleRotation.Roll(),
						pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().x(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().y(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().z());
				}
				else if (pEmitterComponent->GetRenderMode() == engine::ParticleRenderMode::Billboard)
				{
					auto up = particleTransform.GetRotation().ToMatrix3x3() * cd::Vec3f(0, 1, 0);
					auto vec =  pMainCameraTransform.GetTranslation() - pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos();
					auto right = up.Cross(vec);
					float yaw = atan2f(right.z(), right.x());
					float pitch = atan2f(vec.y(), sqrtf(vec.x() * vec.x() + vec.z() * vec.z())); 
					float roll = atan2f(right.x(), -right.y()); 
					bx::mtxSRT(mtx, particleTransform.GetScale().x(), particleTransform.GetScale().y(), particleTransform.GetScale().z(),
						pitch, yaw, roll,
						pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().x(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().y(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().z());
				}
				bgfx::setTransform(mtx);
				bgfx::setState(state_tristrip);
				if (pEmitterComponent->GetEmitterParticleType() == engine::ParticleType::Sprite)
				{
					constexpr StringCrc spriteParticleSampler("s_texColor");
					bgfx::setTexture(0, GetRenderContext()->GetUniform(spriteParticleSampler), m_particleSpriteTextureHandle);
					bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pEmitterComponent->GetSpriteParticleVertexBufferHandle() });
					bgfx::setIndexBuffer(bgfx::IndexBufferHandle{  pEmitterComponent->GetSpriteParticleIndexBufferHandle() });
				}
				else if (pEmitterComponent->GetEmitterParticleType() == engine::ParticleType::Ribbon)
				{
					bgfx::setBuffer(PT_RIBBON_VERTEX_STAGE, bgfx::DynamicVertexBufferHandle{ pRibbonEmitterComponet->GetRibbonParticlePrePosVertexBufferHandle() }, bgfx::Access::ReadWrite);

					//ribbonCount Uinform
					constexpr StringCrc ribbontCounts(ribbonCount);
					cd::Vec4f allRibbonCount{ static_cast<float>(pEmitterComponent->GetParticlePool().GetParticleMaxCount()* Particle::GetMeshVertexCount<ParticleType::Ribbon>()),
						pEmitterComponent->GetParticlePool().GetParticleMaxCount(),
						0,
						0};
					GetRenderContext()->FillUniform(ribbontCounts, &allRibbonCount, 1);

					//ribbonListUniform
					cd::Vec4f ribbonPosList[300]{};
					for (int i = 0; i < 300; i++)
					{
						if (i >= pEmitterComponent->GetParticlePool().GetParticleMaxCount())
						{
							ribbonPosList[i] = cd::Vec4f(0.0f,0.0f,0.0f,0.0f);
						}
						else
						{
						ribbonPosList[i] =cd::Vec4f(pEmitterComponent->GetParticlePool().GetParticle(i).GetPos().x(),
							pEmitterComponent->GetParticlePool().GetParticle(i).GetPos().y(),
							pEmitterComponent->GetParticlePool().GetParticle(i).GetPos().z()
							, 0.0f);
						}
					}
					constexpr StringCrc maxPosList(ribbonMaxPos);
					GetRenderContext()->FillUniform(maxPosList, &ribbonPosList, 300);
					GetRenderContext()->Dispatch(GetViewID(), RibbonParticleProgramCsCrc, 1U, 1U, 1U);
					//pEmitterComponent->UpdateRibbonPosBuffer();
					constexpr StringCrc ribbonParticleSampler("r_texColor");
					bgfx::setTexture(1, GetRenderContext()->GetUniform(ribbonParticleSampler), m_particleRibbonTextureHandle);
					bgfx::setVertexBuffer(0, bgfx::DynamicVertexBufferHandle{ pRibbonEmitterComponet->GetRibbonParticlePrePosVertexBufferHandle() });
					bgfx::setVertexBuffer(1, bgfx::VertexBufferHandle{ pRibbonEmitterComponet->GetRibbonParticleRemainVertexBufferHandle() });
					bgfx::setIndexBuffer(bgfx::IndexBufferHandle{  pRibbonEmitterComponet->GetRibbonParticleIndexBufferHandle() });
				}
				SetRenderMode(pEmitterComponent->GetRenderMode(), pEmitterComponent->GetEmitterParticleType(),pParticleMaterialComponet);
			}
		}

		constexpr StringCrc emitShapeRangeCrc(shapeRange);
		bgfx::setUniform(GetRenderContext()->GetUniform(emitShapeRangeCrc), &pEmitterComponent->GetEmitterShapeRange(), 1);
		bgfx::setTransform(m_pCurrentSceneWorld->GetTransformComponent(entity)->GetWorldMatrix().begin());
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pEmitterComponent->GetEmitterShapeVertexBufferHandle() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pEmitterComponent->GetEmitterShapeIndexBufferHandle() });
		bgfx::setState(state_lines);

		GetRenderContext()->Submit(GetViewID(), ParticleEmitterShapeProgramCrc);
	}
}

void ParticleRenderer::SetRenderMode(engine::ParticleRenderMode& rendermode, engine::ParticleType type, engine::MaterialComponent* shaderFeature_MaterialCompoent)
{
	if (rendermode == engine::ParticleRenderMode::Mesh)
	{
		if (type == engine::ParticleType::Sprite)
		{
			const ShaderResource* pShaderResource = shaderFeature_MaterialCompoent->GetShaderResource();
			GetRenderContext()->Submit(GetViewID(), pShaderResource->GetHandle());
		}
		else if (type == engine::ParticleType::Ribbon)
		{
			GetRenderContext()->Submit(GetViewID(), RibbonParticleProgramCrc);
		}
	}
	else if (rendermode == engine::ParticleRenderMode::Billboard)
	{
		if (type == engine::ParticleType::Sprite)
		{
			GetRenderContext()->Submit(GetViewID(), WO_BillboardParticleProgramCrc);
		}
		else if (type == engine::ParticleType::Ribbon)
		{
			//GetRenderContext()->Submit(GetViewID(), WO_BillboardParticleProgram);
		}
	}
}

void ParticleRenderer::SetRandomPosState(engine::Particle& particle, cd::Vec3f value, cd::Vec3f randomvalue, bool state)
{
	if (state)
	{
		particle.SetPos(value + randomvalue);
	}
	else
	{
		particle.SetPos(value);
	}
}
void ParticleRenderer::SetRandomVelocityState(engine::Particle& particle, cd::Vec3f value, cd::Vec3f randomvalue, bool state)
{
	if (state)
	{
		particle.SetSpeed(value + randomvalue);
	}
	else
	{
		particle.SetSpeed(value);
	}
}

}