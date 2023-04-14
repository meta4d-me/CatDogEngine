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

namespace
{

constexpr const char* classificationSampler = "s_texClassification";
constexpr const char* distanceSampler = "s_texDistance";
constexpr const char* irradianceSampler = "s_texIrradiance";
constexpr const char* relocationSampler = "s_texRelocation";

// Temporary file path.
constexpr const char *classificationTexture = "ddgi/classification.dds";
constexpr const char *distanceTexture = "ddgi/distance.dds";
constexpr const char *irradianceTexture = "ddgi/irradiance.dds";
constexpr const char *relocationTexture = "ddgi/relocation.dds";

constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;

}

void DDGIRenderer::Init()
{
	bgfx::setViewName(GetViewID(), "WorldRenderer");

	m_pRenderContext->CreateUniform(classificationSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform(distanceSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform(irradianceSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform(relocationSampler, bgfx::UniformType::Sampler);

	m_pRenderContext->CreateTexture(classificationTexture, samplerFlags);
	m_pRenderContext->CreateTexture(distanceTexture, samplerFlags);
	m_pRenderContext->CreateTexture(irradianceTexture, samplerFlags);
	m_pRenderContext->CreateTexture(relocationTexture, samplerFlags);
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

		bgfx::setTexture(1, m_pRenderContext->GetUniform(StringCrc(classificationSampler)),
						 m_pRenderContext->GetTexture(StringCrc(classificationTexture)));
		bgfx::setTexture(2, m_pRenderContext->GetUniform(StringCrc(distanceSampler)),
						 m_pRenderContext->GetTexture(StringCrc(distanceTexture)));
		bgfx::setTexture(3, m_pRenderContext->GetUniform(StringCrc(irradianceSampler)),
						 m_pRenderContext->GetTexture(StringCrc(irradianceTexture)));
		bgfx::setTexture(4, m_pRenderContext->GetUniform(StringCrc(relocationSampler)),
						 m_pRenderContext->GetTexture(StringCrc(relocationTexture)));

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		bgfx::submit(GetViewID(), bgfx::ProgramHandle(pMaterialComponent->GetShadingProgram()));
	}
}

void DDGIRenderer::UpdateClassificationTexture(const char *path)
{
	m_pRenderContext->Destory(StringCrc(classificationTexture));
	m_pRenderContext->CreateTexture(classificationTexture, samplerFlags);

}

void DDGIRenderer::UpdateDistanceTexture(const char *path)
{
	m_pRenderContext->Destory(StringCrc(distanceTexture));
	m_pRenderContext->CreateTexture(distanceTexture, samplerFlags);
}

void DDGIRenderer::UpdateIrradianceTexture(const char *path)
{
	m_pRenderContext->Destory(StringCrc(irradianceTexture));
	m_pRenderContext->CreateTexture(irradianceTexture, samplerFlags);
}

void DDGIRenderer::UpdateRelocationTexture(const char *path)
{
	m_pRenderContext->Destory(StringCrc(relocationTexture));
	m_pRenderContext->CreateTexture(relocationTexture, samplerFlags);
}

}
