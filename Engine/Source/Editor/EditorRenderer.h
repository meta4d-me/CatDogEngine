#pragma once

#include "Rendering/Renderer.h"

namespace editor
{

class EditorRenderer final : public engine::Renderer
{
public:
	using Renderer::Renderer;
	virtual ~EditorRenderer();

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

private:
	bgfx::TextureHandle m_imguiFontTexture;
	bgfx::VertexLayout m_imguiVertexLayout;
};

}