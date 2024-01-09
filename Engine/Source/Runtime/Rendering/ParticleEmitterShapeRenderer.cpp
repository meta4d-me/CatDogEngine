#include "ParticleEmitterShapeRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"

namespace engine
{

void ParticleEmitterShapeRenderer::Init()
{
	constexpr StringCrc programCrc = StringCrc("ParticleEmitterShapeProgram");
	GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_particleEmitterShape", "fs_particleEmitterShape" });

	bgfx::setViewName(GetViewID(), "ParticleEmitterShapeRenderer");
}

void ParticleEmitterShapeRenderer::Warmup()
{
	GetRenderContext()->UploadShaderProgram("ParticleEmitterShapeProgram");
}

void ParticleEmitterShapeRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void ParticleEmitterShapeRenderer::Render(float deltaTime)
{
	for (Entity entity : m_pCurrentSceneWorld->GetParticleEmitterEntities())
	{
		auto* pParticleEmitterComponent = m_pCurrentSceneWorld->GetParticleEmitterComponent(entity);
		if (auto* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}

		pParticleEmitterComponent->RePaddingShapeBuffer();
		const bgfx::Memory *pParticleVertexBuffer = bgfx::makeRef(pParticleEmitterComponent->GetEmitterShapeVertexBuffer().data(), static_cast<uint32_t>(pParticleEmitterComponent->GetEmitterShapeVertexBuffer().size()));
		const bgfx::Memory *pParticleIndexBuffer = bgfx::makeRef(pParticleEmitterComponent->GetEmitterShapeIndexBuffer().data(), static_cast<uint32_t>(pParticleEmitterComponent->GetEmitterShapeIndexBuffer().size()));
		bgfx::update(bgfx::DynamicVertexBufferHandle{ pParticleEmitterComponent->GetEmitterShapeVertexBufferHandle()}, 0, pParticleVertexBuffer);
		bgfx::update(bgfx::DynamicIndexBufferHandle{pParticleEmitterComponent->GetEmitterShapeIndexBufferHandle()}, 0, pParticleIndexBuffer);

		bgfx::setVertexBuffer(0, bgfx::DynamicVertexBufferHandle{ pParticleEmitterComponent->GetEmitterShapeVertexBufferHandle() });
		bgfx::setIndexBuffer(bgfx::DynamicIndexBufferHandle{ pParticleEmitterComponent->GetEmitterShapeIndexBufferHandle() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;
		bgfx::setState(state);

		GetRenderContext()->Submit(GetViewID(), "ParticleEmitterShapeProgram");
	}
}

}