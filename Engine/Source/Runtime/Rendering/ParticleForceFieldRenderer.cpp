#include "ParticleForceFieldRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"

namespace engine
{

void ParticleForceFieldRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ParticleForceFieldProgram", "vs_particleforcefield", "fs_particleforcefield"));

	bgfx::setViewName(GetViewID(), "ParticleForceFieldRenderer");
}

void ParticleForceFieldRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void ParticleForceFieldRenderer::Render(float deltaTime)
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

		constexpr StringCrc programHandleIndex{ "ParticleForceFieldProgram" };
		GetRenderContext()->Submit(GetViewID(), programHandleIndex);
	}
}

}