#pragma once

#include "MeshRenderData.h"
#include "Renderer.h"

#include <vector>

namespace engine
{

class FlybyCamera;

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

private:
	bgfx::ProgramHandle m_programPBR;

	RenderDataContext m_renderDataContext;

	bgfx::VertexLayout m_vertexLayout;
	std::vector<MeshHandle> m_meshHandles;
	std::vector<PBRMaterialHandle> m_materialHandles;
};

}