#include "RenderContext.h"

#include "Core/Hashers/StringHash.hpp"
#include "GBuffer.h"
#include "SwapChain.h"

#include <bgfx/bgfx.h>
#include <bimg/decode.h>
#include <bx/allocator.h>

#include <cassert>
#include <format>
#include <fstream>
#include <memory>

namespace
{

static bx::AllocatorI* GetResourceAllocator()
{
	static bx::DefaultAllocator s_allocator;
	return &s_allocator;
}

static void imageReleaseCb(void* _ptr, void* _userData)
{
	BX_UNUSED(_ptr);
	bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)_userData;
	bimg::imageFree(imageContainer);
}

}

namespace engine
{

RenderContext::~RenderContext()
{
	bgfx::shutdown();
}

void RenderContext::Init()
{
	bgfx::Init initDesc;
	initDesc.type = bgfx::RendererType::Direct3D11;
	bgfx::init(initDesc);

	bgfx::setDebug(BGFX_DEBUG_NONE);
}

void RenderContext::Shutdown()
{
}

void RenderContext::BeginFrame()
{
}

void RenderContext::EndFrame()
{
	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	bgfx::frame();
}

uint16_t RenderContext::CreateView()
{
	assert(m_currentViewCount < MaxViewCount && "Overflow the max count of views.");
	return m_currentViewCount++;
}

uint8_t RenderContext::CreateSwapChain(void* pWindowHandle, uint16_t width, uint16_t height)
{
	assert(m_currentSwapChainCount < MaxSwapChainCount && "Overflow the max count of swap chains.");
	m_pSwapChains[m_currentSwapChainCount] = std::make_unique<SwapChain>(pWindowHandle, width, height);
	return m_currentSwapChainCount++;
}

SwapChain* RenderContext::GetSwapChain(uint8_t swapChainID) const
{
	assert(m_pSwapChains[swapChainID] != nullptr && "Invalid swap chain.");
	return m_pSwapChains[swapChainID].get();
}

void RenderContext::InitGBuffer(uint16_t width, uint16_t height)
{
	m_pGBuffer = std::make_unique<GBuffer>(width, height);
	bgfx::reset(m_pGBuffer->GetWidth(), m_pGBuffer->GetHeight(), BGFX_RESET_MSAA_X16 | BGFX_RESET_VSYNC);
}

GBuffer* RenderContext::GetGBuffer() const
{
	return m_pGBuffer.get();
}

bgfx::ShaderHandle RenderContext::CreateShader(const char* pFilePath)
{
	size_t hashValue = Hasher::string_hash(pFilePath);
	auto itShaderCache = m_shaderHandleCaches.find(hashValue);
	if(itShaderCache != m_shaderHandleCaches.end())
	{
		return itShaderCache->second;
	}

	std::string shaderFileFullPath = std::format("{}Shaders/{}", CDENGINE_RESOURCES_ROOT_PATH, pFilePath);
	std::ifstream fin(shaderFileFullPath, std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		return bgfx::ShaderHandle(bgfx::kInvalidHandle);
	}

	fin.seekg(0L, std::ios::end);
	size_t fileSize = fin.tellg();
	fin.seekg(0L, std::ios::beg);
	uint8_t* pRawData = new uint8_t[fileSize];
	fin.read(reinterpret_cast<char*>(pRawData), fileSize);
	fin.close();

	const bgfx::Memory* pMemory = bgfx::makeRef(pRawData, static_cast<uint32_t>(fileSize));
	bgfx::ShaderHandle handle = bgfx::createShader(pMemory);

	if(bgfx::isValid(handle))
	{
		bgfx::setName(handle, pFilePath);
		m_shaderHandleCaches[hashValue] = handle;
	}

	return handle;
}

bgfx::TextureHandle RenderContext::CreateTexture(const char* pFilePath, uint64_t flags)
{
	size_t hashValue = Hasher::string_hash(pFilePath);
	auto itTextureCache = m_textureHandleCaches.find(hashValue);
	if (itTextureCache != m_textureHandleCaches.end())
	{
		return itTextureCache->second;
	}

	std::string textureFileFullPath = std::format("{}Textures/{}", CDENGINE_RESOURCES_ROOT_PATH, pFilePath);
	std::ifstream fin(textureFileFullPath, std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		return bgfx::TextureHandle(bgfx::kInvalidHandle);
	}

	fin.seekg(0L, std::ios::end);
	size_t fileSize = fin.tellg();
	fin.seekg(0L, std::ios::beg);
	uint8_t* pRawData = new uint8_t[fileSize];
	fin.read(reinterpret_cast<char*>(pRawData), fileSize);
	fin.close();

	bimg::ImageContainer* imageContainer = bimg::imageParse(GetResourceAllocator(), pRawData, static_cast<uint32_t>(fileSize));
	const bgfx::Memory* mem = bgfx::makeRef(
		imageContainer->m_data
		, imageContainer->m_size
		, imageReleaseCb
		, imageContainer
	);

	delete[] pRawData;
	pRawData = nullptr;

	bgfx::TextureHandle handle(bgfx::kInvalidHandle);
	if (imageContainer->m_cubeMap)
	{
		handle = bgfx::createTextureCube(
			uint16_t(imageContainer->m_width)
			, 1 < imageContainer->m_numMips
			, imageContainer->m_numLayers
			, bgfx::TextureFormat::Enum(imageContainer->m_format)
			, flags
			, mem
		);
	}
	else if (1 < imageContainer->m_depth)
	{
		handle = bgfx::createTexture3D(
			uint16_t(imageContainer->m_width)
			, uint16_t(imageContainer->m_height)
			, uint16_t(imageContainer->m_depth)
			, 1 < imageContainer->m_numMips
			, bgfx::TextureFormat::Enum(imageContainer->m_format)
			, flags
			, mem
		);
	}
	else if (bgfx::isTextureValid(0, false, imageContainer->m_numLayers, bgfx::TextureFormat::Enum(imageContainer->m_format), flags))
	{
		handle = bgfx::createTexture2D(
			uint16_t(imageContainer->m_width)
			, uint16_t(imageContainer->m_height)
			, 1 < imageContainer->m_numMips
			, imageContainer->m_numLayers
			, bgfx::TextureFormat::Enum(imageContainer->m_format)
			, flags
			, mem
		);
	}

	if (bgfx::isValid(handle))
	{
		bgfx::setName(handle, pFilePath);
		m_textureHandleCaches[hashValue] = handle;
	}

	return handle;
}

bgfx::UniformHandle RenderContext::CreateUniform(const char* pName, bgfx::UniformType::Enum uniformType, uint16_t number)
{
	size_t hashValue = Hasher::string_hash(pName);
	auto itUniformCache = m_uniformHandleCaches.find(hashValue);
	if (itUniformCache != m_uniformHandleCaches.end())
	{
		return itUniformCache->second;
	}

	bgfx::UniformHandle uniformHandle = bgfx::createUniform(pName, uniformType, number);
	if(bgfx::isValid(uniformHandle))
	{
		m_uniformHandleCaches[hashValue] = uniformHandle;
	}

	return uniformHandle;
}

}