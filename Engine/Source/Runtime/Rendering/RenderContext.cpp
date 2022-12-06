#include "RenderContext.h"

#include "Display/Camera.h"
#include "Renderer.h"

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
	for (auto it : m_programHandleCaches)
	{
		bgfx::destroy(it.second);
	}

	for(auto it : m_shaderHandleCaches)
	{
		bgfx::destroy(it.second);
	}

	for (auto it : m_textureHandleCaches)
	{
		bgfx::destroy(it.second);
	}

	for (auto it : m_uniformHandleCaches)
	{
		bgfx::destroy(it.second);
	}
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

void RenderContext::OnResize(uint16_t width, uint16_t height)
{
	bgfx::reset(width, height, BGFX_RESET_MSAA_X16 | BGFX_RESET_VSYNC);
}

uint16_t RenderContext::CreateView()
{
	assert(m_currentViewCount < MaxViewCount && "Overflow the max count of views.");
	return m_currentViewCount++;
}

RenderTarget* RenderContext::CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, std::vector<AttachmentDescriptor> attachmentDescs)
{
	return CreateRenderTarget(resourceCrc, std::make_unique<RenderTarget>(width, height, std::move(attachmentDescs)));
}

RenderTarget* RenderContext::CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, void* pWindowHandle)
{
	return CreateRenderTarget(resourceCrc, std::make_unique<RenderTarget>(width, height, pWindowHandle));
}

RenderTarget* RenderContext::CreateRenderTarget(StringCrc resourceCrc, std::unique_ptr<RenderTarget> pRenderTarget)
{
	auto itResourceCache = m_renderTargetCaches.find(resourceCrc.value());
	if (itResourceCache != m_renderTargetCaches.end())
	{
		return itResourceCache->second.get();
	}

	assert(m_renderTargetCaches.size() <= MaxRenderTargetCount && "Overflow the max count of render targets.");

	m_renderTargetCaches[resourceCrc.value()] = std::move(pRenderTarget);
	return m_renderTargetCaches[resourceCrc.value()].get();
}

bgfx::ShaderHandle RenderContext::CreateShader(const char* pFilePath)
{
	StringCrc filePath(pFilePath);
	auto itShaderCache = m_shaderHandleCaches.find(filePath.value());
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
		m_shaderHandleCaches[filePath.value()] = handle;
	}

	return handle;
}

bgfx::ProgramHandle RenderContext::CreateProgram(const char* pName, const char* pVSName, const char* pFSName)
{
	return CreateProgram(pName, CreateShader(pVSName), CreateShader(pFSName));
}

bgfx::ProgramHandle RenderContext::CreateProgram(const char* pName, bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh)
{
	StringCrc programName(pName);
	auto itProgram = m_programHandleCaches.find(programName.value());
	if (itProgram != m_programHandleCaches.end())
	{
		return itProgram->second;
	}

	bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh);
	if(bgfx::isValid(program))
	{
		m_programHandleCaches[programName.value()] = program;
	}

	return program;
}

bgfx::ProgramHandle RenderContext::CreateProgram(const char *pName, const char *pCSName)
{
	return CreateProgram(pName, CreateShader(pCSName));
}

bgfx::ProgramHandle RenderContext::CreateProgram(const char *pName, bgfx::ShaderHandle csh)
{
	StringCrc programName(pName);
	auto itProgram = m_programHandleCaches.find(programName.value());
	if (itProgram != m_programHandleCaches.end())
	{
		return itProgram->second;
	}

	bgfx::ProgramHandle program = bgfx::createProgram(csh, true);
	if (bgfx::isValid(program))
	{
		m_programHandleCaches[programName.value()] = program;
	}

	return program;
}

bgfx::TextureHandle RenderContext::CreateTexture(const char* pFilePath, uint64_t flags)
{
	StringCrc filePath(pFilePath);
	auto itTextureCache = m_textureHandleCaches.find(filePath.value());
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
		m_textureHandleCaches[filePath.value()] = handle;
	}

	return handle;
}

bgfx::TextureHandle RenderContext::CreateTexture(const char* pName, const uint16_t _width, const uint16_t _height, uint64_t flags)
{
	StringCrc textureName(pName);
	auto itTextureCache = m_textureHandleCaches.find(textureName.value());
	if (itTextureCache != m_textureHandleCaches.end())
	{
		return itTextureCache->second;
	}

	bgfx::TextureHandle texture = bgfx::createTexture2D(_width, _height, false, 1, bgfx::TextureFormat::RGBA32F, flags);
	if (bgfx::isValid(texture))
	{
		bgfx::setName(texture, pName);
		m_textureHandleCaches[textureName.value()] = texture;
	}

	return texture;
}

bgfx::TextureHandle RenderContext::CreateTexture(const char *pName, const uint16_t _width, const uint16_t _height, const uint16_t _depth, uint64_t flags)
{
	StringCrc textureName(pName);
	auto itTextureCache = m_textureHandleCaches.find(textureName.value());
	if (itTextureCache != m_textureHandleCaches.end())
	{
		return itTextureCache->second;
	}

	bgfx::TextureHandle texture = bgfx::createTexture3D(_width, _height, _depth, false, bgfx::TextureFormat::RGBA32F, flags);
	if (bgfx::isValid(texture))
	{
		bgfx::setName(texture, pName);
		m_textureHandleCaches[textureName.value()] = texture;
	}

	return texture;
}

bgfx::UniformHandle RenderContext::CreateUniform(const char* pName, bgfx::UniformType::Enum uniformType, uint16_t number)
{
	StringCrc uniformName(pName);
	auto itUniformCache = m_uniformHandleCaches.find(uniformName.value());
	if (itUniformCache != m_uniformHandleCaches.end())
	{
		return itUniformCache->second;
	}

	bgfx::UniformHandle uniformHandle = bgfx::createUniform(pName, uniformType, number);
	if(bgfx::isValid(uniformHandle))
	{
		m_uniformHandleCaches[uniformName.value()] = uniformHandle;
	}

	return uniformHandle;
}

void RenderContext::SetVertexLayout(StringCrc resourceCrc, bgfx::VertexLayout vertexLayoutHandle)
{
	auto itVertexLayoutCache = m_vertexLayoutCaches.find(resourceCrc.value());
	if (itVertexLayoutCache != m_vertexLayoutCaches.end())
	{
		return;
	}

	m_vertexLayoutCaches[resourceCrc.value()] = std::move(vertexLayoutHandle);
}

void RenderContext::SetTexture(StringCrc resourceCrc, bgfx::TextureHandle textureHandle)
{
	auto itTextureCache = m_textureHandleCaches.find(resourceCrc.value());
	if (itTextureCache != m_textureHandleCaches.end())
	{
		return;
	}

	m_textureHandleCaches[resourceCrc.value()] = std::move(textureHandle);
}

RenderTarget* RenderContext::GetRenderTarget(StringCrc resourceCrc) const
{
	auto itResource = m_renderTargetCaches.find(resourceCrc.value());
	if (itResource != m_renderTargetCaches.end())
	{
		return itResource->second.get();
	}

	return nullptr;
}

const bgfx::VertexLayout& RenderContext::GetVertexLayout(StringCrc resourceCrc) const
{
	auto itResource = m_vertexLayoutCaches.find(resourceCrc.value());
	if (itResource != m_vertexLayoutCaches.end())
	{
		return itResource->second;
	}

	static bgfx::VertexLayout dummy;
	return dummy;
}

const bgfx::ShaderHandle& RenderContext::GetShader(StringCrc resourceCrc) const
{
	auto itResource = m_shaderHandleCaches.find(resourceCrc.value());
	if (itResource != m_shaderHandleCaches.end())
	{
		return itResource->second;
	}

	static bgfx::ShaderHandle dummy(bgfx::kInvalidHandle);
	return dummy;
}

const bgfx::ProgramHandle& RenderContext::GetProgram(StringCrc resourceCrc) const
{
	auto itResource = m_programHandleCaches.find(resourceCrc.value());
	if (itResource != m_programHandleCaches.end())
	{
		return itResource->second;
	}

	static bgfx::ProgramHandle dummy(bgfx::kInvalidHandle);
	return dummy;
}

const bgfx::TextureHandle& RenderContext::GetTexture(StringCrc resourceCrc) const
{
	auto itResource = m_textureHandleCaches.find(resourceCrc.value());
	if (itResource != m_textureHandleCaches.end())
	{
		return itResource->second;
	}

	static bgfx::TextureHandle dummy(bgfx::kInvalidHandle);
	return dummy;
}

const bgfx::UniformHandle& RenderContext::GetUniform(StringCrc resourceCrc) const
{
	auto itResource = m_uniformHandleCaches.find(resourceCrc.value());
	if (itResource != m_uniformHandleCaches.end())
	{
		return itResource->second;
	}

	static bgfx::UniformHandle dummy(bgfx::kInvalidHandle);
	return dummy;
}

}