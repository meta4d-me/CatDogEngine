#include "DebugRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

//#include <format>

namespace engine
{

void DebugRenderer::Init()
{
	GetRenderContext()->CreateProgram("WireFrameProgram", "vs_debug.bin", "fs_debug.bin");
	bgfx::setViewName(GetViewID(), "DebugRenderer");
}

void DebugRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void DebugRenderer::Render(float deltaTime)
{
	for (Entity entity : m_pCurrentSceneWorld->GetStaticMeshEntities())
	{
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

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pMeshComponent->GetVertexBuffer() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pMeshComponent->GetIndexBuffer() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA );// | BGFX_STATE_PT_LINESTRIP
		bgfx::setState(state);

		constexpr StringCrc wireframeProgram("WireFrameProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(wireframeProgram));
	}
}

}