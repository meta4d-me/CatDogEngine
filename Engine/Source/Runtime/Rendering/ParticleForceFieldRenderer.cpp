#include "ParticleForceFieldRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"

namespace engine
{

void ParticleForceFieldRenderer::Init()
{
	constexpr StringCrc programCrc = StringCrc("ParticleForceFieldProgram");
	GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_particleforcefield", "fs_particleforcefield" });

	bgfx::setViewName(GetViewID(), "ParticleForceFieldRenderer");
}

void ParticleForceFieldRenderer::Warmup()
{
	GetRenderContext()->UploadShaderProgram("ParticleForceFieldProgram");
}

void ParticleForceFieldRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void ParticleForceFieldRenderer::Render(float deltaTime)
{
	for (Entity entity : m_pCurrentSceneWorld->GetParticleForceFieldEntities())
	{
		if (auto* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}
		auto* pParticleForceFieldComponent = m_pCurrentSceneWorld->GetParticleForceFieldComponent(entity);
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pParticleForceFieldComponent->GetVertexBufferHandle() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pParticleForceFieldComponent->GetIndexBufferHandle() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;
		bgfx::setState(state);

		GetRenderContext()->Submit(GetViewID(), "ParticleForceFieldProgram");
	}
}

}