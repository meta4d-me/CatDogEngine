#include "GBuffer.h"

#include <bgfx/bgfx.h>

namespace engine
{

GBuffer::GBuffer(uint16_t width, uint16_t height)
{
	Resize(width, height);
}

void GBuffer::Resize(uint16_t width, uint16_t height)
{
	if (width == m_width && height == m_height)
	{
		return;
	}

	if (!m_pFrameBufferHandle)
	{
		m_pFrameBufferHandle = std::make_unique<bgfx::FrameBufferHandle>();
	}
	else
	{
		// When creating the frame buffer, we specified the flag to auto release textures.
		bgfx::destroy(*m_pFrameBufferHandle.get());
	}

	const uint64_t tsFlags = BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	m_backBuffers[0] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA32F, tsFlags);
	m_backBuffers[1] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA32F, tsFlags);
	m_backBuffers[2] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D32F, tsFlags);
	*m_pFrameBufferHandle = bgfx::createFrameBuffer(3, m_backBuffers, true);

	m_width = width;
	m_height = height;
}

}