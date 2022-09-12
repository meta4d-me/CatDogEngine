#pragma once

#include <inttypes.h>

namespace bgfx
{

struct FrameBufferHandle;

}

namespace engine
{

// The name "SwapChain" may not be suitable for OpenGL APIs..
// A renderer should bind with a swapchain to display rendering surface in a window.
class SwapChain
{
public:
	SwapChain() = delete;
	explicit SwapChain(void* pWindowHandle, uint16_t width, uint16_t height);
	SwapChain(const SwapChain&) = delete;
	SwapChain& operator=(const SwapChain&) = delete;
	SwapChain(SwapChain&&) = delete;
	SwapChain& operator=(SwapChain&&) = delete;
	~SwapChain();

	uint16_t GetWidth() const { return m_frameBufferWidth; }
	uint16_t GetHeight() const { return m_frameBufferHeight; }
	const bgfx::FrameBufferHandle* GetFrameBuffer() const { return m_pFrameBufferHandle; }
	float GetAspect() const { return static_cast<float>(m_frameBufferWidth) / static_cast<float>(m_frameBufferHeight);}
	void Resize(uint16_t width, uint16_t height);

private:
	uint16_t					m_frameBufferWidth;
	uint16_t					m_frameBufferHeight;

	// Use pointer because I don't want to any files include SwapChain.h to introduce bgfx.h
	// If you think out a better way, please reach us~
	bgfx::FrameBufferHandle*	m_pFrameBufferHandle = nullptr;
};

}