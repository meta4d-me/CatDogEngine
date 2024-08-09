#include "GaussianRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"

void engine::GaussianRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("GaussianProgram", "vs_gaussianSplatting", "fs_gaussianSplatting"));

	bgfx::setViewName(GetViewID(), "GaussianRenderer");
	// NOTE: CreateTexture
	//const bgfx::Memory* mem = bgfx::copy(texturedata, sizeof(texturedata));
	//bgfx::TextureHandle texture = bgfx::createTexture(mem, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE);


	//NOTE : CreateTexture2D
	//uint16_t width = 1024;
	//uint16_t height = 1024; 
	//bool hasMips = false;  //if-mip-map
	//uint16_t numLayers = 1;
	//bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8; // fomat

	//const bgfx::Memory* mem = bgfx::copy(texturedata, sizeof(texturedata));
	//bgfx::TextureHandle texture = bgfx::createTexture2D(width, height, hasMips, numLayers, format, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, mem);
}

void engine::GaussianRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void engine::GaussianRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	for (Entity entity : m_pCurrentSceneWorld->GetGaussianRenderEntities())
	{
		if (auto* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}
		auto* pGaussianComponent = m_pCurrentSceneWorld->GetGaussianRenderComponent(entity);
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pGaussianComponent->GetVertexBufferHandle() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pGaussianComponent->GetIndexBufferHandle() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;
		bgfx::setState(state);

		constexpr StringCrc programHandleIndex{ "GaussianProgram" };
		GetRenderContext()->Submit(GetViewID(), programHandleIndex);
	}
}
