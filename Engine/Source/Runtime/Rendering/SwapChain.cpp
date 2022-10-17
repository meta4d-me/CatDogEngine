#include "SwapChain.h"

#include <bgfx/bgfx.h>

namespace engine
{

SwapChain::SwapChain(void* pWindowHandle, uint16_t width, uint16_t height)
{
	m_frameBufferWidth = width;
	m_frameBufferHeight = height;

	m_pFrameBufferHandle = std::make_unique<bgfx::FrameBufferHandle>();
	*m_pFrameBufferHandle = bgfx::createFrameBuffer(pWindowHandle, width, height);
}

void SwapChain::Resize(uint16_t width, uint16_t height)
{
	width;
	height;
	// How to let bgfx know it is the target swap chain?
	// bgfx::reset(width, height, BGFX_RESET_VSYNC);
}

}