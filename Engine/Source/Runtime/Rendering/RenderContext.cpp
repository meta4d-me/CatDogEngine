#include "RenderContext.h"

#include "GBuffer.h"
#include "SwapChain.h"

#include <bgfx/bgfx.h>

#include <cassert>
#include <memory>

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
	m_pSwapChains[m_currentSwapChainCount] = std::make_unique<SwapChain>(pWindowHandle, width, height);
	return m_currentSwapChainCount++;
}

SwapChain* RenderContext::GetSwapChain(uint8_t swapChainID) const
{
	assert(m_pSwapChains[swapChainID] != nullptr && "Invalid swap chain.");
	return m_pSwapChains[swapChainID].get();
}

void RenderContext::InitGBuffer(uint16_t width, uint16_t height)
{
	m_pGBuffer = std::make_unique<GBuffer>(width, height);
	bgfx::reset(m_pGBuffer->GetWidth(), m_pGBuffer->GetHeight(), BGFX_RESET_MSAA_X16 | BGFX_RESET_VSYNC);
}

GBuffer* RenderContext::GetGBuffer() const
{
	return m_pGBuffer.get();
}

}