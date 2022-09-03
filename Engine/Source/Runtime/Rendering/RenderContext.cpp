#include "RenderContext.h"

#include <bgfx/platform.h>

namespace engine::Rendering
{

RenderContext::RenderContext(uint16_t width, uint16_t height, void* pWindowPtr)
{
	bgfx::PlatformData pd;
	pd.nwh = pWindowPtr;
	bgfx::setPlatformData(pd);
	bgfx::renderFrame();

	m_backBufferWidth = width;
	m_backBufferHeight = height;
}

RenderContext::~RenderContext()
{
	bgfx::shutdown();
}

void RenderContext::Init()
{
	bgfx::Init bgfxInit;
	bgfxInit.type = bgfx::RendererType::Direct3D11;
	bgfxInit.resolution.width = m_backBufferWidth;
	bgfxInit.resolution.height = m_backBufferHeight;
	bgfxInit.resolution.reset = BGFX_RESET_VSYNC;
	bgfx::init(bgfxInit);

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