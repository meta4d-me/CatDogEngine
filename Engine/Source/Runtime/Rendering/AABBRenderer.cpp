#include "AABBRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/ShaderVariantCollections.h"
#include "Scene/Texture.h"

namespace engine
{

void AABBRenderer::Init()
{
	constexpr StringCrc programCrc = StringCrc("AABBProgram");
	GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_AABB", "fs_AABB" });

	bgfx::setViewName(GetViewID(), "AABBRenderer");
}

void AABBRenderer::Warmup()
{
	GetRenderContext()->UploadShaderProgram("AABBProgram");
}

void AABBRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void AABBRenderer::Render(float deltaTime)
{
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
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pCollisionMesh->GetVertexBuffer() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pCollisionMesh->GetIndexBuffer() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;
		bgfx::setState(state);

		GetRenderContext()->Submit(GetViewID(), "AABBProgram");
	}
}

}