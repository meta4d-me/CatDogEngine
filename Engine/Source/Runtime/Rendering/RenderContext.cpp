#include "RenderContext.h"

#include "Log/Log.h"

#include "Renderer.h"
#include "Rendering/DDGIDefinition.h"
#include "Rendering/Utility/VertexLayoutUtility.h"

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

template<class T>
void DestoryImpl(engine::StringCrc resourceCrc, T &caches) {
	auto itResource = caches.find(resourceCrc.Value());
	if(itResource != caches.end()) {
		bgfx::destroy(itResource->second);
		caches.erase(itResource);
	}
};

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
	auto itResourceCache = m_renderTargetCaches.find(resourceCrc.Value());
	if (itResourceCache != m_renderTargetCaches.end())
	{
		return itResourceCache->second.get();
	}

	assert(m_renderTargetCaches.size() <= MaxRenderTargetCount && "Overflow the max count of render targets.");

	m_renderTargetCaches[resourceCrc.Value()] = std::move(pRenderTarget);
	return m_renderTargetCaches[resourceCrc.Value()].get();
}

bgfx::ShaderHandle RenderContext::CreateShader(const char* pFilePath)
{
	StringCrc filePath(pFilePath);
	auto itShaderCache = m_shaderHandleCaches.find(filePath.Value());
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
		m_shaderHandleCaches[filePath.Value()] = handle;
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
	auto itProgram = m_programHandleCaches.find(programName.Value());
	if (itProgram != m_programHandleCaches.end())
	{
		return itProgram->second;
	}

	bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh);
	if(bgfx::isValid(program))
	{
		m_programHandleCaches[programName.Value()] = program;
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
	auto itProgram = m_programHandleCaches.find(programName.Value());
	if (itProgram != m_programHandleCaches.end())
	{
		return itProgram->second;
	}

	bgfx::ProgramHandle program = bgfx::createProgram(csh, true);
	if (bgfx::isValid(program))
	{
		m_programHandleCaches[programName.Value()] = program;
	}

	return program;
}

bgfx::TextureHandle RenderContext::CreateTexture(const char* pFilePath, uint64_t flags)
{
	StringCrc filePath(pFilePath);
	auto itTextureCache = m_textureHandleCaches.find(filePath.Value());
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
		m_textureHandleCaches[filePath.Value()] = handle;
	}

	return handle;
}

bgfx::TextureHandle RenderContext::CreateTexture(const char* pName, uint16_t width, uint16_t height, uint16_t depth, bgfx::TextureFormat::Enum formet, uint64_t flags, const void* data, uint32_t size)
{
	StringCrc textureName(pName);
	auto itTextureCache = m_textureHandleCaches.find(textureName.Value());
	if(itTextureCache != m_textureHandleCaches.end())
	{
		return itTextureCache->second;
	}

	const bgfx::Memory *mem = nullptr;
	bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;

	if(nullptr != data && 0 != size)
	{
		mem = bgfx::makeRef(data, size);
	}

	if(depth > 1)
	{
		texture = nullptr == mem ?
			bgfx::createTexture3D(width, height, depth, false, formet, flags) :
			bgfx::createTexture3D(width, height, depth, false, formet, flags, mem);
	}
	else
	{
		texture = nullptr == mem ?
			bgfx::createTexture2D(width, height, false, 1, formet, flags) :
			bgfx::createTexture2D(width, height, false, 1, formet, flags, mem);
	}

	if(bgfx::isValid(texture))
	{
		bgfx::setName(texture, pName);
		m_textureHandleCaches[textureName.Value()] = texture;
	}
	else
	{
		CD_ENGINE_ERROR("Faild to create texture {0}!", pName);
	}

	return texture;
}

bgfx::UniformHandle RenderContext::CreateUniform(const char* pName, bgfx::UniformType::Enum uniformType, uint16_t number)
{
	StringCrc uniformName(pName);
	auto itUniformCache = m_uniformHandleCaches.find(uniformName.Value());
	if (itUniformCache != m_uniformHandleCaches.end())
	{
		return itUniformCache->second;
	}

	bgfx::UniformHandle uniformHandle = bgfx::createUniform(pName, uniformType, number);
	if(bgfx::isValid(uniformHandle))
	{
		m_uniformHandleCaches[uniformName.Value()] = uniformHandle;
	}

	return uniformHandle;
}

bgfx::VertexLayout RenderContext::CreateVertexLayout(StringCrc resourceCrc, const std::vector<cd::VertexAttributeLayout>& vertexAttributes)
{
	auto itVertexLayoutCache = m_vertexLayoutCaches.find(resourceCrc.Value());
	if (itVertexLayoutCache != m_vertexLayoutCaches.end())
	{
		return itVertexLayoutCache->second;
	}

	bgfx::VertexLayout newVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(newVertexLayout, vertexAttributes);
	m_vertexLayoutCaches[resourceCrc.Value()] = newVertexLayout;
	return newVertexLayout;
}

bgfx::VertexLayout RenderContext::CreateVertexLayout(StringCrc resourceCrc, const cd::VertexAttributeLayout& vertexAttribute)
{
	auto itVertexLayoutCache = m_vertexLayoutCaches.find(resourceCrc.Value());
	if (itVertexLayoutCache != m_vertexLayoutCaches.end())
	{
		return itVertexLayoutCache->second;
	}

	bgfx::VertexLayout newVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(newVertexLayout, vertexAttribute);
	m_vertexLayoutCaches[resourceCrc.Value()] = newVertexLayout;
	return newVertexLayout;
}

void RenderContext::SetVertexLayout(StringCrc resourceCrc, bgfx::VertexLayout vertexLayoutHandle)
{
	m_vertexLayoutCaches[resourceCrc.Value()] = std::move(vertexLayoutHandle);
}

void RenderContext::SetTexture(StringCrc resourceCrc, bgfx::TextureHandle textureHandle)
{
	m_textureHandleCaches[resourceCrc.Value()] = std::move(textureHandle);
}

void RenderContext::SetUniform(StringCrc resourceCrc, bgfx::UniformHandle uniformreHandle)
{
	m_uniformHandleCaches[resourceCrc.Value()] = std::move(uniformreHandle);
}

void RenderContext::FillUniform(StringCrc resourceCrc, const void *pData, uint16_t vec4Count) const
{
	bgfx::setUniform(GetUniform(resourceCrc), pData, vec4Count);
}

RenderTarget* RenderContext::GetRenderTarget(StringCrc resourceCrc) const
{
	auto itResource = m_renderTargetCaches.find(resourceCrc.Value());
	if (itResource != m_renderTargetCaches.end())
	{
		return itResource->second.get();
	}

	return nullptr;
}

const bgfx::VertexLayout& RenderContext::GetVertexLayout(StringCrc resourceCrc) const
{
	auto itResource = m_vertexLayoutCaches.find(resourceCrc.Value());
	if (itResource != m_vertexLayoutCaches.end())
	{
		return itResource->second;
	}

	static bgfx::VertexLayout dummy;
	return dummy;
}

bgfx::ShaderHandle RenderContext::GetShader(StringCrc resourceCrc) const
{
	auto itResource = m_shaderHandleCaches.find(resourceCrc.Value());
	if (itResource != m_shaderHandleCaches.end())
	{
		return itResource->second;
	}

	return bgfx::ShaderHandle(bgfx::kInvalidHandle);
}

bgfx::ProgramHandle RenderContext::GetProgram(StringCrc resourceCrc) const
{
	auto itResource = m_programHandleCaches.find(resourceCrc.Value());
	if (itResource != m_programHandleCaches.end())
	{
		return itResource->second;
	}

	return bgfx::ProgramHandle(bgfx::kInvalidHandle);
}

bgfx::TextureHandle RenderContext::GetTexture(StringCrc resourceCrc) const
{
	auto itResource = m_textureHandleCaches.find(resourceCrc.Value());
	if (itResource != m_textureHandleCaches.end())
	{
		return itResource->second;
	}

	return bgfx::TextureHandle(bgfx::kInvalidHandle);
}

bgfx::UniformHandle RenderContext::GetUniform(StringCrc resourceCrc) const
{
	auto itResource = m_uniformHandleCaches.find(resourceCrc.Value());
	if (itResource != m_uniformHandleCaches.end())
	{
		return itResource->second;
	}

	return bgfx::UniformHandle(bgfx::kInvalidHandle);
}

void RenderContext::Destory(StringCrc resourceCrc)
{
	DestoryImpl(resourceCrc, m_shaderHandleCaches);
	DestoryImpl(resourceCrc, m_programHandleCaches);
	DestoryImpl(resourceCrc, m_textureHandleCaches);
	DestoryImpl(resourceCrc, m_uniformHandleCaches);
}

}