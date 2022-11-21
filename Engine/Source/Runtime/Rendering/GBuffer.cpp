#include "GBuffer.h"

#include <bgfx/bgfx.h>

namespace engine
{

GBuffer::GBuffer(uint16_t width, uint16_t height)
{
	m_frameBufferWidth = width;
	m_frameBufferHeight = height;

	m_pFrameBufferHandle = std::make_unique<bgfx::FrameBufferHandle>();
	const uint64_t tsFlags = 0
		| BGFX_TEXTURE_RT
		| BGFX_SAMPLER_U_CLAMP
		| BGFX_SAMPLER_V_CLAMP;

	m_backBuffers[0] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA32F, tsFlags);
	m_backBuffers[1] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA32F, tsFlags);
	m_backBuffers[2] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D32F, tsFlags);
	*m_pFrameBufferHandle = bgfx::createFrameBuffer(3, m_backBuffers, true);
}

}