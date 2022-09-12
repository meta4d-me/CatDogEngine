#pragma once

#include <string>

#include <bgfx/bgfx.h>

namespace engine
{

class SwapChain;

class Renderer
{
public:
	Renderer() = delete;
	explicit Renderer(uint16_t viewID, SwapChain* pSwapChain);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	virtual ~Renderer() {}

	virtual void Init() = 0;
	virtual void Render(float deltaTime);

	uint16_t GetViewID() const { return m_viewID; }
	const SwapChain* GetSwapChain() const { return m_pSwapChain; } 

public:
	static bgfx::ShaderHandle LoadShader(std::string fileName);
	static bgfx::TextureHandle LoadTexture(std::string filePath);

protected:
	uint16_t		m_viewID = 0;
	SwapChain*		m_pSwapChain = nullptr;
};

}