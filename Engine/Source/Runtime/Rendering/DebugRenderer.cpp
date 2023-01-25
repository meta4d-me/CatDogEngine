#include "DebugRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ECWorld/World.h"
#include "Display/Camera.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

#include <format>

namespace engine
{

void DebugRenderer::Init()
{
	m_pRenderContext->CreateProgram("WireFrameProgram", "vs_wireframe.bin", "fs_wireframe.bin");
	bgfx::setViewName(GetViewID(), "DebugRenderer");
}

void DebugRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void DebugRenderer::Render(float deltaTime)
{
	auto pMeshStorage = m_pCurrentWorld->GetComponents<StaticMeshComponent>();
	auto pTransformStorage = m_pCurrentWorld->GetComponents<TransformComponent>();

	for (Entity entity : pMeshStorage->GetEntities())
	{
		StaticMeshComponent* pMeshComponent = pMeshStorage->GetComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		if (TransformComponent* pTransformComponent = pTransformStorage->GetComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle(pMeshComponent->GetAABBVertexBuffer()));
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle(pMeshComponent->GetAABBIndexBuffer()));

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		bgfx::setState(state);

		constexpr StringCrc wireframeProgram("WireFrameProgram");
		bgfx::submit(GetViewID(), m_pRenderContext->GetProgram(wireframeProgram));
	}
}

}