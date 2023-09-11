#include "AABBRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

namespace engine
{

void AABBRenderer::Init()
{
	GetRenderContext()->CreateProgram("AABBProgram", "vs_AABB.bin", "fs_AABB.bin");
	bgfx::setViewName(GetViewID(), "AABBRenderer");
}

void AABBRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void AABBRenderer::RenderLines(uint32_t entity)
{
	auto* pCollisionMesh = m_pCurrentSceneWorld->GetCollisionMeshComponent(static_cast<Entity>(entity));
	if (!pCollisionMesh)
	{
		return;
	}

	if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(static_cast<Entity>(entity)))
	{
		pTransformComponent->Build();
		bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
	}

	bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pCollisionMesh->GetVertexBuffer() });
	bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pCollisionMesh->GetIndexBuffer() });

	constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
		BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;
	bgfx::setState(state);

	constexpr StringCrc AABBProgram("AABBProgram");
	bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(AABBProgram));
}

void AABBRenderer::Render(float deltaTime)
{
	if (m_isRenderSelected) 
	{
		Entity entity = m_pCurrentSceneWorld->GetSelectedEntity();
		RenderLines(static_cast<uint32_t>(entity));
	}
	else
	{
		for (Entity entity : m_pCurrentSceneWorld->GetStaticMeshEntities())
		{
			RenderLines(static_cast<uint32_t>(entity));
		}
	}
}

}