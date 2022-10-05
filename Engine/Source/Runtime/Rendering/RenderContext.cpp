#include "RenderContext.h"

#include "GBuffer.h"
#include "SwapChain.h"

#include <bgfx/bgfx.h>

#include <cassert>

namespace engine
{

RenderContext::~RenderContext()
{
	bgfx::shutdown();
}

void RenderContext::Init()
{
	bgfx::Init initDesc;
	initDesc.type = bgfx::RendererType::Direct3D11;
	bgfx::init(initDesc);

	bgfx::setDebug(BGFX_DEBUG_NONE);
}

void RenderContext::Shutdown()
{
	for(uint8_t swapChainIndex = 0; swapChainIndex < MaxSwapChainCount; ++swapChainIndex)
	{
		if(SwapChain* pSwapChain = m_pSwapChains[swapChainIndex])
		{
			delete pSwapChain;
			m_pSwapChains[swapChainIndex] = nullptr;
		}
	}

	if (m_pGBuffer)
	{
		delete m_pGBuffer;
		m_pGBuffer = nullptr;
	}
}

void RenderContext::BeginFrame()
{
}

void RenderContext::EndFrame()
{
	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	bgfx::frame();
}

uint16_t RenderContext::CreateView()
{
	assert(m_currentViewCount < MaxViewCount && "Overflow the max count of views.");
	return m_currentViewCount++;
}

uint8_t RenderContext::CreateSwapChain(void* pWindowHandle, uint16_t width, uint16_t height)
{
	assert(m_currentSwapChainCount < MaxSwapChainCount && "Overflow the max count of swap chains.");
	m_pSwapChains[m_currentSwapChainCount] = new SwapChain(pWindowHandle, width, height);
	return m_currentSwapChainCount++;
}

SwapChain* RenderContext::GetSwapChain(uint8_t swapChainID) const
{
	assert(m_pSwapChains[swapChainID] != nullptr && "Invalid swap chain.");
	return m_pSwapChains[swapChainID];
}

void RenderContext::InitGBuffer(uint16_t width, uint16_t height)
{
	m_pGBuffer = new GBuffer(width, height);
	bgfx::reset(m_pGBuffer->GetWidth(), m_pGBuffer->GetHeight(), BGFX_RESET_MSAA_X16 | BGFX_RESET_VSYNC);
}

GBuffer* RenderContext::GetGBuffer() const
{
	return m_pGBuffer;
}

}