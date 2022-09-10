#pragma once

#include <inttypes.h>

namespace engine
{

class RenderContext
{
public:
	RenderContext() = default;
	~RenderContext();

	void Init(uint16_t width, uint16_t height, void* pWindowPtr);
	void BeginFrame();
	void EndFrame();

	void ResizeSwapChain(uint16_t width, uint16_t height);
	uint16_t GetWidth() const { return m_backBufferWidth; }
	uint16_t GetHeight() const { return m_backBufferHeight; }

private:
	uint16_t		m_backBufferWidth;
	uint16_t		m_backBufferHeight;
};

}