#pragma once

#include <inttypes.h>
#include <memory>

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

	uint16_t GetWidth() const { return m_frameBufferWidth; }
	uint16_t GetHeight() const { return m_frameBufferHeight; }
	float GetAspect() const { return static_cast<float>(m_frameBufferWidth) / static_cast<float>(m_frameBufferHeight); }
	const bgfx::FrameBufferHandle* GetFrameBuffer() const { return m_pFrameBufferHandle.get(); }

private:
	uint16_t m_frameBufferWidth;
	uint16_t m_frameBufferHeight;
	std::unique_ptr<bgfx::FrameBufferHandle> m_pFrameBufferHandle;
};

}