#pragma once

#include "Renderer.h"

namespace engine
{

class BlitRenderTargetPass final : public Renderer
{
public:
	using Renderer::Renderer;
	virtual ~BlitRenderTargetPass();

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

private:
	uint16_t m_blitTextureWidth;
	uint16_t m_blitTextureHeight;
};

}