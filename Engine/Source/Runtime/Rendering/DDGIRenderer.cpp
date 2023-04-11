#include "DDGIRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Material/ShaderSchema.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

namespace engine
{

void DDGIRenderer::Init()
{
	constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;

	bgfx::setViewName(GetViewID(), "WorldRenderer");
}

void DDGIRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bgfx::setViewFrameBuffer(GetViewID(),* GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void DDGIRenderer::Render(float deltaTime)
{
	for(Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
	{
		MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		if(!pMaterialComponent || pMaterialComponent->GetMaterialType() != m_pCurrentSceneWorld->GetDDGIMaterialType())
		{
			continue;
		}

		// No mesh attached
		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if(!pMeshComponent)
		{
			continue;
		}

		// non-DDGI
		if(!m_pCurrentSceneWorld->GetDDGIComponent(entity))
		{
			continue;
		}

		// Transform
		if(TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		// Mesh
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle(pMeshComponent->GetVertexBuffer()));
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle(pMeshComponent->GetIndexBuffer()));

		for(const auto& [textureType, textureInfo] : pMaterialComponent->GetTextureResources())
		{
			std::optional<MaterialComponent::TextureInfo> optTextureInfo = pMaterialComponent->GetTextureInfo(textureType);
			if(optTextureInfo.has_value())
			{
				const MaterialComponent::TextureInfo& textureInfo = optTextureInfo.value();
				bgfx::setTexture(textureInfo.slot, bgfx::UniformHandle(textureInfo.samplerHandle), bgfx::TextureHandle(textureInfo.textureHandle));
			}
		}

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		bgfx::submit(GetViewID(), bgfx::ProgramHandle(pMaterialComponent->GetShadingProgram()));
	}
}

}
