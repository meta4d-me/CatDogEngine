#include "GBuffer.h"

#include <bgfx/bgfx.h>

namespace engine
{

GBuffer::GBuffer(uint16_t width, uint16_t height)
{
	m_frameBufferWidth = width;
	m_frameBufferHeight = height;

	m_pFrameBufferHandle = new bgfx::FrameBufferHandle();
	const uint64_t tsFlags = 0
		| BGFX_TEXTURE_RT
		| BGFX_SAMPLER_U_CLAMP
		| BGFX_SAMPLER_V_CLAMP;

	bgfx::TextureHandle gbufferTex[] =
	{
		bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA32F, tsFlags),
		bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA32F, tsFlags),
		bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D32F, tsFlags),
	};
	*m_pFrameBufferHandle = bgfx::createFrameBuffer(3, gbufferTex, true);
}

GBuffer::~GBuffer()
{
	if(m_pFrameBufferHandle)
	{
		bgfx::destroy(*m_pFrameBufferHandle);
		delete m_pFrameBufferHandle;
		m_pFrameBufferHandle = nullptr;
	}
}

void GBuffer::Resize(uint16_t width, uint16_t height)
{
	width;
	height;
}

}