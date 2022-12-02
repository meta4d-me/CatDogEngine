#pragma once

#include "MeshRenderData.h"
#include "Renderer.h"

#include <vector>

namespace engine
{

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
		TextureHandle metalness;
		TextureHandle roughness;
		TextureHandle emissive;
		TextureHandle ao;
	};

public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

private:
	bgfx::ProgramHandle m_programPBR;

	RenderDataContext m_renderDataContext;

	std::vector<MeshHandle> m_meshHandles;
	std::vector<PBRMaterialHandle> m_materialHandles;
};

}