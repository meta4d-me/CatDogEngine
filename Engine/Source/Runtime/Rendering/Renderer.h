#pragma once

#include <bgfx/bgfx.h>

#include <string>

namespace engine
{

class GBuffer;
class SwapChain;
class FlybyCamera;

class Renderer
{
public:
	Renderer() = delete;
	explicit Renderer(uint16_t viewID, SwapChain* pSwapChain, GBuffer* pGBuffer, FlybyCamera* pCamera);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	virtual ~Renderer() = default;

	virtual void Init() = 0;
	virtual void UpdateView() = 0;
	virtual void Render(float deltaTime);

	uint16_t GetViewID() const { return m_viewID; }
	const SwapChain* GetSwapChain() const { return m_pSwapChain; } 
	const GBuffer* GetGBuffer() const { return m_pGBuffer; }

public:
	static bgfx::ShaderHandle LoadShader(std::string fileName);
	static bgfx::TextureHandle LoadTexture(std::string filePath, uint64_t flags = 0UL);
	static void ScreenSpaceQuad(float _textureWidth, float _textureHeight, bool _originBottomLeft = false, float _width = 1.0f, float _height = 1.0f);

protected:
	uint16_t		m_viewID = 0;
	SwapChain*		m_pSwapChain = nullptr;
	GBuffer*		m_pGBuffer = nullptr;
	FlybyCamera*	m_pFlybyCamera = nullptr;
};

}