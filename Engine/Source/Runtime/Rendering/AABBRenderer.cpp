#include "AABBRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Scene/Texture.h"

namespace engine
{

void AABBRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("AABBProgram", "vs_AABB", "fs_AABB"));

	bgfx::setViewName(GetViewID(), "AABBRenderer");
}

void AABBRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void AABBRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	for (Entity entity : m_pCurrentSceneWorld->GetCollisionMeshEntities())
	{
		auto* pCollisionMesh = m_pCurrentSceneWorld->GetCollisionMeshComponent(entity);
		if (!m_enableGlobalAABB && !pCollisionMesh->IsDebugDrawEnable())
		{
			continue;
		}

		if (auto* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pCollisionMesh->GetVertexBuffer() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pCollisionMesh->GetIndexBuffer() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;
		bgfx::setState(state);

		constexpr StringCrc programHandleIndex{ "AABBProgram" };
		GetRenderContext()->Submit(GetViewID(), programHandleIndex);
	}
}

}