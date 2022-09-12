#include "RenderContext.h"

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

	bgfx::setDebug(BGFX_DEBUG_PROFILER);
}

void RenderContext::Shutdown()
{
	for(uint8_t swapChainIndex = 0; swapChainIndex < MaxSwapChainCount; ++swapChainIndex)
	{
		if(SwapChain* pSwapChain = m_swapChains[swapChainIndex])
		{
			delete pSwapChain;
			m_swapChains[swapChainIndex] = nullptr;
		}
	}
}

void RenderContext::BeginFrame()
{
}

void RenderContext::EndFrame()
{
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
	m_swapChains[m_currentSwapChainCount] = new SwapChain(pWindowHandle, width, height);
	return m_currentSwapChainCount++;
}

SwapChain* RenderContext::GetSwapChain(uint8_t swapChainID) const
{
	assert(m_swapChains[swapChainID] != nullptr && "Invalid swap chain.");
	return m_swapChains[swapChainID];
}

}