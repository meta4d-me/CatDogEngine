#include "TextureResource.h"

#include "Log/Log.h"
#include "Resources/ResourceLoader.h"
#include "Scene/Texture.h"

#include <bgfx/bgfx.h>
#include <bimg/decode.h>
#include <bx/allocator.h>

#include <format>

namespace details
{

static bx::AllocatorI* GetResourceAllocator()
{
	static bx::DefaultAllocator s_allocator;
	return &s_allocator;
}

bgfx::TextureHandle BGFXCreateTexture(
	uint16_t width,
	uint16_t height,
	uint16_t depth,
	bool isCubeMap,
	bool hasMips,
	uint16_t numLayers,
	bgfx::TextureFormat::Enum textureFormat,
	uint64_t textureFlags,
	const bgfx::Memory* pMemory)
{
	bgfx::TextureHandle textureHandle = BGFX_INVALID_HANDLE;
	if (isCubeMap)
	{
		textureHandle = bgfx::createTextureCube(width, hasMips, numLayers, textureFormat, textureFlags, pMemory);
	}
	else if (depth > 1)
	{
		textureHandle = bgfx::createTexture3D(width, height, depth, hasMips, textureFormat, textureFlags, pMemory);
	}
	else if (bgfx::isTextureValid(0, false, numLayers, textureFormat, textureFlags))
	{
		textureHandle = bgfx::createTexture2D(width, height, hasMips, numLayers, textureFormat, textureFlags, pMemory);
	}
	return textureHandle;
}

}

namespace engine
{

TextureResource::TextureResource() = default;

TextureResource::~TextureResource()
{
	// Collect garbage intermediatly.
	SetStatus(ResourceStatus::Garbage);
	Update();
}

void TextureResource::SetTextureAsset(const cd::Texture* pTextureAsset)
{
	m_pTextureAsset = pTextureAsset;
}

void TextureResource::SetDDSBuiltTexturePath(std::string ddsFilePath)
{
	m_ddsFilePath = cd::MoveTemp(ddsFilePath);
}

void TextureResource::UpdateTextureType(cd::MaterialPropertyGroup textureType)
{
	m_enableSRGB = cd::MaterialPropertyGroup::BaseColor == textureType || cd::MaterialPropertyGroup::Emissive == textureType;
}

void TextureResource::UpdateUVMapMode(cd::TextureMapMode u, cd::TextureMapMode v)
{
	m_uvMapMode[0] = u;
	m_uvMapMode[1] = v;
}

void TextureResource::Update()
{
	switch (GetStatus())
	{
	case ResourceStatus::Loading:
	{
		// TODO : Texture seems not to need to get data from cd::Texture now.
		if (!m_ddsFilePath.empty())
		{
			// TODO : build texture
			//m_textureRawData = engine::ResourceLoader::LoadFile(m_pTextureAsset->GetPath());
			m_textureRawData = engine::ResourceLoader::LoadFile(m_ddsFilePath.c_str());
			SetStatus(ResourceStatus::Loaded);
		}
		break;
	}
	case ResourceStatus::Loaded:
	{
		if (!m_textureRawData.empty())
		{
			SetStatus(ResourceStatus::Building);
		}
		break;
	}
	case ResourceStatus::Building:
	{
		m_textureImageData = bimg::imageParse(details::GetResourceAllocator(), m_textureRawData.data(), static_cast<uint32_t>(m_textureRawData.size()));
		SetStatus(ResourceStatus::Built);
		break;
	}
	case ResourceStatus::Built:
	{
		if (m_textureImageData != nullptr)
		{
			BuildSamplerHandle();
			BuildTextureHandle();
			m_recycleCount = 0U;
			SetStatus(ResourceStatus::Ready);
		}
		break;
	}
	case ResourceStatus::Ready:
	{
		// Release CPU data later to save memory.
		constexpr uint32_t recycleDelayFrames = 30U;
		if (m_recycleCount++ >= recycleDelayFrames)
		{
			ClearTextureData();

			m_recycleCount = 0U;
			SetStatus(ResourceStatus::Optimized);
		}
		break;
	}
	case ResourceStatus::Garbage:
	{
		DestroySamplerHandle();
		DestroyTextureHandle();
		// CPU data will destroy after deconstructor.
		SetStatus(ResourceStatus::Destroyed);
		break;
	}
	default:
		break;
	}
}

void TextureResource::Reset()
{
	DestroySamplerHandle();
	DestroyTextureHandle();
	FreeTextureData();
	SetStatus(ResourceStatus::Loading);
}

uint64_t TextureResource::GetTextureFlags() const
{
	uint64_t textureFlags = m_enableSRGB ? BGFX_TEXTURE_SRGB : 0;
	switch (m_uvMapMode[0])
	{
	case cd::TextureMapMode::Clamp:
		textureFlags |= BGFX_SAMPLER_U_CLAMP;
		break;
	case cd::TextureMapMode::Mirror:
		textureFlags |= BGFX_SAMPLER_U_MIRROR;
		break;
	case cd::TextureMapMode::Border:
		textureFlags |= BGFX_SAMPLER_U_BORDER;
		break;
	case cd::TextureMapMode::Wrap:
	default:
		break;
	}

	switch (m_uvMapMode[1])
	{
	case cd::TextureMapMode::Clamp:
		textureFlags |= BGFX_SAMPLER_V_CLAMP;
		break;
	case cd::TextureMapMode::Mirror:
		textureFlags |= BGFX_SAMPLER_V_MIRROR;
		break;
	case cd::TextureMapMode::Border:
		textureFlags |= BGFX_SAMPLER_V_BORDER;
		break;
	case cd::TextureMapMode::Wrap:
	default:
		break;
	}

	return textureFlags;
}

void TextureResource::BuildSamplerHandle()
{
	assert(m_samplerHandle == UINT16_MAX);
	static int textureIndex = 0;
	std::string samplerUniformName = std::format("s_textureSampler{}", textureIndex++);
	m_samplerHandle = bgfx::createUniform(samplerUniformName.c_str(), bgfx::UniformType::Sampler).idx;
	assert(m_samplerHandle != UINT16_MAX);
}

void TextureResource::BuildTextureHandle()
{
	assert(m_textureHandle == UINT16_MAX);
	auto* pImageContainer = reinterpret_cast<bimg::ImageContainer*>(m_textureImageData);
	const bgfx::Memory* pImageContent = bgfx::makeRef(pImageContainer->m_data, pImageContainer->m_size);
	m_textureHandle = details::BGFXCreateTexture(pImageContainer->m_width, pImageContainer->m_height, pImageContainer->m_depth, false, pImageContainer->m_numMips > 1,
		1, static_cast<bgfx::TextureFormat::Enum>(pImageContainer->m_format), GetTextureFlags(), pImageContent).idx;
	assert(m_textureHandle != UINT16_MAX);
}

void TextureResource::ClearTextureData()
{
	m_textureRawData.clear();
	if (m_textureImageData)
	{
		auto* pImageContainer = reinterpret_cast<bimg::ImageContainer*>(m_textureImageData);
		bimg::imageFree(pImageContainer);
		m_textureImageData = nullptr;
	}
}

void TextureResource::FreeTextureData()
{
	ClearTextureData();
	TextureRawData().swap(m_textureRawData);
}

void TextureResource::DestroySamplerHandle()
{
	if (m_samplerHandle != UINT16_MAX)
	{
		bgfx::destroy(bgfx::UniformHandle{ m_samplerHandle });
		m_samplerHandle = UINT16_MAX;
	}
}

void TextureResource::DestroyTextureHandle()
{
	if (m_textureHandle != UINT16_MAX)
	{
		bgfx::destroy(bgfx::TextureHandle{ m_textureHandle });
		m_textureHandle = UINT16_MAX;
	}
}

}