#include "RenderTarget.h"

#include <bgfx/bgfx.h>

namespace engine
{

RenderTarget::RenderTarget(uint16_t width, uint16_t height, void* hwnd) :
	m_hwnd(hwnd)
{
	Resize(width, height);
}

RenderTarget::RenderTarget(uint16_t width, uint16_t height, std::vector<AttachmentDescriptor> attachmentDescs) :
	m_attachmentDescriptors(std::move(attachmentDescs))
{
	Resize(width, height);
}

bgfx::TextureHandle RenderTarget::GetTextureHandle(int index) const
{
	return bgfx::getTexture(*m_pFrameBufferHandle.get(), index);
}

void RenderTarget::Resize(uint16_t width, uint16_t height)
{
	if (width == m_width && height == m_height)
	{
		return;
	}

	m_width = width;
	m_height = height;

	if (!m_pFrameBufferHandle)
	{
		m_pFrameBufferHandle = std::make_unique<bgfx::FrameBufferHandle>();
	}
	else
	{
		// When creating the frame buffer, we specified the flag to auto release textures.
		bgfx::destroy(*m_pFrameBufferHandle.get());
	}

	if (IsSwapChainTarget())
	{
		*m_pFrameBufferHandle = bgfx::createFrameBuffer(m_hwnd, width, height);
	}
	else
	{
		std::vector<bgfx::TextureHandle> textureHandles;
		textureHandles.reserve(m_attachmentDescriptors.size());
		uint64_t msaaFlag = 1ULL << BGFX_TEXTURE_RT_MSAA_SHIFT;
		for (const auto& attachmentDescriptor : m_attachmentDescriptors)
		{
			uint64_t textureFlags = 0ULL;
			bgfx::TextureFormat::Enum textureFormat;
			switch (attachmentDescriptor.textureFormat)
			{
			case TextureFormat::D32F:
				textureFormat = bgfx::TextureFormat::D32F;
				textureFlags |= BGFX_TEXTURE_RT_WRITE_ONLY | msaaFlag;
				break;
			case TextureFormat::RGBA32F:
			default:
				textureFormat = bgfx::TextureFormat::RGBA32F;
				textureFlags |= BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_TEXTURE_MSAA_SAMPLE | msaaFlag;
				break;
			};
			textureHandles.push_back(bgfx::createTexture2D(width, height, false, 1, textureFormat, textureFlags));
		}
		*m_pFrameBufferHandle = bgfx::createFrameBuffer(static_cast<uint8_t>(textureHandles.size()), textureHandles.data(), true);
	}

	OnResize.Invoke(m_width, m_height);
}

}