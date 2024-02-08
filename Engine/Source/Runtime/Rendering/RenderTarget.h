#pragma once

#include "Core/Delegates/MulticastDelegate.hpp"

#include <bgfx/bgfx.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace bgfx
{

struct FrameBufferHandle;

}

namespace engine
{

enum class TextureFormat
{
	RGBA8,
	RGBA16F,
	RGBA32F,
	D32F
};

struct AttachmentDescriptor
{
	TextureFormat textureFormat;
};

class RenderTarget
{
public:
	RenderTarget() = delete;
	explicit RenderTarget(uint16_t width, uint16_t height, void* hwnd);
	explicit RenderTarget(uint16_t width, uint16_t height, std::vector<AttachmentDescriptor> attachmentDescs);
	RenderTarget(const RenderTarget&) = delete;
	RenderTarget& operator=(const RenderTarget&) = delete;
	RenderTarget(RenderTarget&&) = delete;
	RenderTarget& operator=(RenderTarget&&) = delete;
	~RenderTarget() = default;

	bool IsSwapChainTarget() const { return m_hwnd != nullptr && m_attachmentDescriptors.empty(); }
	uint16_t GetWidth() const { return m_width; }
	uint16_t GetHeight() const { return m_height; }
	void Resize(uint16_t width, uint16_t height);
	float GetAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

	const bgfx::FrameBufferHandle* GetFrameBufferHandle() const { return m_pFrameBufferHandle.get(); }
	bgfx::TextureHandle GetTextureHandle(int index) const;

public:
	MulticastDelegate<void(uint16_t, uint16_t)> OnResize;

private:
	uint16_t m_width = 0;
	uint16_t m_height = 0;
	void* m_hwnd = nullptr;
	std::vector<AttachmentDescriptor> m_attachmentDescriptors;

	std::unique_ptr<bgfx::FrameBufferHandle> m_pFrameBufferHandle;
};

}