#include "RenderContext.h"

#include <bgfx/platform.h>

namespace engine
{

RenderContext::~RenderContext()
{
	bgfx::shutdown();
}

void RenderContext::Init(uint16_t width, uint16_t height, void* pWindowPtr)
{
	m_backBufferWidth = width;
	m_backBufferHeight = height;

	bgfx::Init initDesc;
	initDesc.type = bgfx::RendererType::Direct3D11;
	initDesc.platformData.nwh = pWindowPtr;
	initDesc.resolution.width = width;
	initDesc.resolution.height = height;
	initDesc.resolution.reset = BGFX_RESET_VSYNC;
	bgfx::init(initDesc);

	bgfx::setDebug(BGFX_DEBUG_TEXT);
}

void RenderContext::ResizeSwapChain(uint16_t width, uint16_t height)
{
	bgfx::reset(width, height, BGFX_RESET_VSYNC);
}

void RenderContext::BeginFrame()
{

}

void RenderContext::EndFrame()
{
	bgfx::frame();
}

}