#pragma once

#include "bgfx/bgfx.h"

#include <string>

namespace engine::Rendering
{

class RenderContext;

class Renderer
{
public:
	Renderer() = delete;
	Renderer(RenderContext* pRenderContext);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	virtual ~Renderer() {}

	virtual void Init() = 0;
	virtual void Render(float deltaTime) = 0;

	void SetViewID(uint16_t viewId) { m_viewId = viewId; }
	uint16_t GetViewID() const { return m_viewId; }

	RenderContext* GetRenderContext() const { return m_pRenderContext; }

public:
	static bgfx::ShaderHandle LoadShader(std::string fileName);
	static bgfx::TextureHandle LoadTexture(std::string filePath);

protected:
	uint16_t		m_viewId = 0;
	uint16_t		m_viewWidth = 1;
	uint16_t		m_viewHeight = 1;
	RenderContext*  m_pRenderContext = nullptr;
};

}