#pragma once

//#include <bgfx/bgfx.h>

#include <string>

namespace engine
{

class Camera;
class GBuffer;
class RenderContext;
class SwapChain;

class Renderer
{
public:
	Renderer() = delete;
	explicit Renderer(RenderContext* pRenderContext, uint16_t viewID, SwapChain* pSwapChain);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	virtual ~Renderer() = default;

	virtual void Init() = 0;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) = 0;
	virtual void Render(float deltaTime) = 0;

	uint16_t GetViewID() const { return m_viewID; }
	const SwapChain* GetSwapChain() const { return m_pSwapChain; } 
	const GBuffer* GetGBuffer() const { return m_pGBuffer; }

	void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }
	Camera* GetCamera() const { return m_pCamera; }

public:
	static void ScreenSpaceQuad(float _textureWidth, float _textureHeight, bool _originBottomLeft = false, float _width = 1.0f, float _height = 1.0f);

protected:
	uint16_t		m_viewID = 0;
	RenderContext*	m_pRenderContext = nullptr;
	SwapChain*		m_pSwapChain = nullptr;
	GBuffer*		m_pGBuffer = nullptr;
	Camera*			m_pCamera = nullptr;
};

}