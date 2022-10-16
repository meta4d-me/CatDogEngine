#pragma once

#include "MeshRenderData.h"
#include "Renderer.h"

#include <vector>

namespace engine
{

class SkyRenderer;

class SceneRenderer final : public Renderer
{
private:
	struct MeshHandle
	{
	public:
		bgfx::VertexBufferHandle vbh;
		bgfx::IndexBufferHandle ibh;
	};

	struct TextureHandle
	{
		bgfx::UniformHandle sampler;
		bgfx::TextureHandle texture;
	};

	struct PBRMaterialHandle
	{
	public:
		TextureHandle baseColor;
		TextureHandle normal;
		TextureHandle orm;
	};

public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView() override;
	virtual void Render(float deltaTime) override;

	void SetDependentRender(const SkyRenderer* pRenderer) { m_pSkyRenderer = pRenderer; }

private:
	bgfx::ProgramHandle m_programPBR;

	RenderDataContext m_renderDataContext;

	bgfx::VertexLayout m_vertexLayout;
	std::vector<MeshHandle> m_meshHandles;
	std::vector<PBRMaterialHandle> m_materialHandles;

	// TODO : As different renderers have resource dependencies.
	// I need to make a solution for this case:
	// 1.A resource manager holds every resource handles in bgfx. Any renderers can use string to find the expected one.
	// 2.As RenderGraph talk said, I need to split resource compile stage away from render logic!
	// So here is the workaround to render correctly.
	const SkyRenderer* m_pSkyRenderer = nullptr;
};

}