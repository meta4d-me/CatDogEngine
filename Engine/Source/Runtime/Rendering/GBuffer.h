#pragma once

#include <inttypes.h>
#include <memory>

#include <bgfx/bgfx.h>

namespace bgfx
{

struct FrameBufferHandle;

}

namespace engine
{

class GBuffer
{
public:
	GBuffer() = delete;
	explicit GBuffer(uint16_t width, uint16_t height);
	GBuffer(const GBuffer&) = delete;
	GBuffer& operator=(const GBuffer&) = delete;
	GBuffer(GBuffer&&) = delete;
	GBuffer& operator=(GBuffer&&) = delete;
	~GBuffer() = default;

	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }
	void Resize(uint16_t width, uint16_t height);
	float GetAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
	const bgfx::FrameBufferHandle* GetFrameBuffer() const { return m_pFrameBufferHandle.get(); }
	bgfx::TextureHandle GetBackBufferTextureHandle(int index) const { return m_backBuffers[index]; }

private:
	uint16_t m_width;
	uint16_t m_height;
	std::unique_ptr<bgfx::FrameBufferHandle> m_pFrameBufferHandle;

	bgfx::TextureHandle m_backBuffers[3];
};

}