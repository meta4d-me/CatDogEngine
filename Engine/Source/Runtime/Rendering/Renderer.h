#pragma once

//#include <bgfx/bgfx.h>

#include <string>

namespace engine
{

class Camera;
class RenderContext;
class RenderTarget;

class Renderer
{
public:
	Renderer() = delete;
	explicit Renderer(RenderContext* pRenderContext, uint16_t viewID, RenderTarget* pRenderTarget);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	virtual ~Renderer() = default;

	virtual void Init() = 0;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) = 0;
	virtual void Render(float deltaTime) = 0;

	uint16_t GetViewID() const { return m_viewID; }
	const RenderTarget* GetRenderTarget() const { return m_pRenderTarget; }

	void Enable() { m_isEnable = true; }
	void Disable() { m_isEnable = false; }
	bool IsEnable() const { return m_isEnable; }

public:
	static void ScreenSpaceQuad(float _textureWidth, float _textureHeight, bool _originBottomLeft = false, float _width = 1.0f, float _height = 1.0f);

protected:
	uint16_t		m_viewID = 0;
	RenderContext*	m_pRenderContext = nullptr;
	RenderTarget*	m_pRenderTarget = nullptr;
	bool			m_isEnable = true;
};

}