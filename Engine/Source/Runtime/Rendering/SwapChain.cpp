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

}