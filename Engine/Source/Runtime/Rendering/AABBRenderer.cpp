#include "AABBRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

namespace engine
{

void AABBAllRenderer::Init()
{
	GetRenderContext()->CreateProgram("AABBAllProgram", "vs_AABB.bin", "fs_AABB.bin");
	bgfx::setViewName(GetViewID(), "AABBAllRenderer");
}

void AABBAllRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void AABBAllRenderer::Render(float deltaTime)
{
	for (Entity entity : m_pCurrentSceneWorld->GetStaticMeshEntities())
	{
		if (m_pCurrentSceneWorld->GetSkyEntity() == entity)
		{
			continue;
		}

		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pMeshComponent->GetAABBVertexBuffer() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pMeshComponent->GetAABBIndexBuffer() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINESTRIP;
		bgfx::setState(state);

		constexpr StringCrc AABBAllProgram("AABBAllProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(AABBAllProgram));

	}
}

void AABBSelectedRenderer::Init()
{
	GetRenderContext()->CreateProgram("AABBProgram", "vs_AABB.bin", "fs_AABB.bin");
	bgfx::setViewName(GetViewID(), "AABBRenderer");
}

void AABBSelectedRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void AABBSelectedRenderer::Render(float deltaTime)
{
	Entity entity = m_pCurrentSceneWorld->GetSelectedEntity();
	if (m_pCurrentSceneWorld->GetSkyEntity() == entity)
	{
		return;
	}

	StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
	if (!pMeshComponent)
	{
		return;
	}

	if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
	{
		pTransformComponent->Build();
		bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
	}

	bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pMeshComponent->GetAABBVertexBuffer() });
	bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pMeshComponent->GetAABBIndexBuffer() });

	constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
		BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINESTRIP;
	bgfx::setState(state);

	constexpr StringCrc AABBProgram("AABBProgram");
	bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(AABBProgram));
}

}