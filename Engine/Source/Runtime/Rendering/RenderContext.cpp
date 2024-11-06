#include "RenderContext.h"

#include "Base/Template.h"
#include "Log/Log.h"
#include "Path/Path.h"
#include "Renderer.h"
#include "Rendering/Resources/ResourceContext.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Rendering/ShaderType.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Resources/ResourceLoader.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bimg/decode.h>
#include <bx/allocator.h>

#include <cassert>
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

void RenderContext::Init(GraphicsBackend backend, void* hwnd)
{
	bgfx::Init initDesc;
	switch (backend)
	{
	case GraphicsBackend::OpenGL:
	{
		initDesc.type = bgfx::RendererType::OpenGL;
		break;
	}
	case GraphicsBackend::OpenGLES:
	{
		initDesc.type = bgfx::RendererType::OpenGLES;
		break;
	}
	case GraphicsBackend::Direct3D11:
	{
		initDesc.type = bgfx::RendererType::Direct3D11;
		break;
	}
	case GraphicsBackend::Direct3D12:
	{
		initDesc.type = bgfx::RendererType::Direct3D12;
		break;
	}
	case GraphicsBackend::Vulkan:
	{
		initDesc.type = bgfx::RendererType::Vulkan;
		break;
	}
	case GraphicsBackend::Metal:
	{
		initDesc.type = bgfx::RendererType::Metal;
		break;
	}
	case GraphicsBackend::Noop:
	default:
	{
		initDesc.type = bgfx::RendererType::Noop;
		break;
	}
	}

	initDesc.platformData.nwh = hwnd;
	bgfx::init(initDesc);
}

void RenderContext::Shutdown()
{
	for (auto it : m_textureHandleCaches)
	{
		bgfx::destroy(bgfx::TextureHandle{ it.second });
	}

	for (auto it : m_uniformHandleCaches)
	{
		bgfx::destroy(bgfx::UniformHandle{ it.second });
	}
}

void RenderContext::BeginFrame()
{
}

void RenderContext::Submit(uint16_t viewID, uint16_t programHandle)
{
	assert(bgfx::isValid(bgfx::ProgramHandle{ programHandle }));
	bgfx::submit(viewID, bgfx::ProgramHandle{ programHandle });
}

void RenderContext::Submit(uint16_t viewID, StringCrc programHandleIndex)
{
	Submit(viewID, m_pResourceContext->GetShaderResource(programHandleIndex)->GetHandle());
}

void RenderContext::Dispatch(uint16_t viewID, uint16_t programHandle, uint32_t numX, uint32_t numY, uint32_t numZ)
{
	assert(bgfx::isValid(bgfx::ProgramHandle{ programHandle }));
	bgfx::dispatch(viewID, bgfx::ProgramHandle{ programHandle }, numX, numY, numZ);
}
void RenderContext::Dispatch(uint16_t viewID, StringCrc programHandleIndex, uint32_t numX, uint32_t numY, uint32_t numZ)
{
	Dispatch(viewID, m_pResourceContext->GetShaderResource(programHandleIndex)->GetHandle(), numX, numY, numZ);
}

void RenderContext::EndFrame()
{
	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	bgfx::frame();
}

void RenderContext::OnResize(uint16_t width, uint16_t height)
{
	bgfx::reset(width, height, BGFX_RESET_VSYNC);
	m_backBufferWidth = width;
	m_backBufferHeight = height;
}

uint16_t RenderContext::CreateView()
{
	assert(m_currentViewCount < MaxViewCount && "Overflow the max count of views.");
	return m_currentViewCount++;
}

ShaderResource* RenderContext::RegisterShaderProgram(const std::string& programName, const std::string& vsName, const std::string& fsName, const std::string& combine)
{
	const StringCrc programCrc{ programName + combine };
	if (m_pResourceContext->GetShaderResource(programCrc))
	{
		return m_pResourceContext->GetShaderResource(programCrc);
	}

	ShaderResource* pShaderResource = m_pResourceContext->AddShaderResource(programCrc);
	pShaderResource->SetType(ShaderProgramType::Standard);
	pShaderResource->SetName(programName);
	pShaderResource->SetShaders(vsName, fsName, combine);

	AddShaderResource(StringCrc{ vsName }, pShaderResource);
	AddShaderResource(StringCrc{ fsName }, pShaderResource);

	return pShaderResource;
}

ShaderResource* RenderContext::RegisterShaderProgram(const std::string& programName, const std::string& shaderName, ShaderProgramType type, const std::string& combine)
{
	StringCrc programCrc{ programName + combine };
	if (m_pResourceContext->GetShaderResource(programCrc))
	{
		return m_pResourceContext->GetShaderResource(programCrc);
	}

	ShaderResource* pShaderResource = m_pResourceContext->AddShaderResource(StringCrc{ programName + combine });
	pShaderResource->SetType(type);
	pShaderResource->SetName(programName);
	pShaderResource->SetShader(ProgramTypeToSingleShaderType.at(type), shaderName, combine);

	AddShaderResource(StringCrc{ shaderName }, pShaderResource);

	return pShaderResource;
}

void RenderContext::OnShaderHotModified(std::string modifiedShaderName)
{
	// Delete all related compiled uber shader file.
	// TODO : Need a file system to check is compiled file dirty.
	for (const auto& entry : std::filesystem::recursive_directory_iterator(Path::GetShaderOutputDirectory()))
	{
		if (!entry.is_regular_file() || entry.path().extension() != Path::ShaderOutputExtension)
		{
			continue;
		}

		std::string fileName = entry.path().filename().generic_string();
		if (fileName.find(modifiedShaderName) != std::string::npos)
		{
			std::filesystem::remove(entry.path());
			CD_ENGINE_TRACE("Delete obsolete file {}", fileName);
		}
	}

	// Get all ShaderResource variants by shader name.
	auto range = m_shaderResources.equal_range(StringCrc{ modifiedShaderName });
	for (auto it = range.first; it != range.second; ++it)
	{
		m_modifiedShaderResources.insert(it->second);
	}
}

void RenderContext::OnShaderRecompile()
{
	// m_modifiedShaderResources will be filled by callback function which bound to FileWatcher.
	auto it = m_modifiedShaderResources.begin();
	while (it != m_modifiedShaderResources.end())
	{
		ShaderResource* pShaderResource = *it;
		if (pShaderResource->IsActive())
		{
			pShaderResource->Reset();
			AddRecompileShaderResource(pShaderResource);

			it = m_modifiedShaderResources.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void RenderContext::AddCompileFailedEntity(uint32_t entity)
{
	m_compileFailedEntities.insert(entity);
}

void RenderContext::ClearCompileFailedEntity()
{
	m_compileFailedEntities.clear();
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
	auto itResourceCache = m_renderTargetCaches.find(resourceCrc);
	if (itResourceCache != m_renderTargetCaches.end())
	{
		return itResourceCache->second.get();
	}

	assert(m_renderTargetCaches.size() <= MaxRenderTargetCount && "Overflow the max count of render targets.");

	m_renderTargetCaches[resourceCrc] = std::move(pRenderTarget);
	return m_renderTargetCaches[resourceCrc].get();
}

bgfx::TextureHandle RenderContext::CreateTexture(const char* pFilePath, uint64_t flags)
{
	StringCrc filePathCrc{ pFilePath };
	auto itTextureCache = m_textureHandleCaches.find(filePathCrc);
	if (itTextureCache != m_textureHandleCaches.end())
	{
		return { itTextureCache->second };
	}

	//std::string textureFileFullPath = std::format("{}{}", CDPROJECT_RESOURCES_ROOT_PATH, pShaderName);
	std::string textureFileFullPath = CDPROJECT_RESOURCES_ROOT_PATH;
	textureFileFullPath += pFilePath;
	std::ifstream fin(textureFileFullPath, std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		return bgfx::TextureHandle{ bgfx::kInvalidHandle };
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

	bgfx::TextureHandle handle{ bgfx::kInvalidHandle };
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
		m_textureHandleCaches[filePathCrc] = handle.idx;
	}

	return handle;
}

bgfx::TextureHandle RenderContext::CreateTexture(const char* pName, uint16_t width, uint16_t height, uint16_t depth, bgfx::TextureFormat::Enum format, uint64_t flags, const void* data, uint32_t size)
{
	StringCrc textureNameCrc{ pName };
	auto itTextureCache = m_textureHandleCaches.find(textureNameCrc);
	if(itTextureCache != m_textureHandleCaches.end())
	{
		return { itTextureCache->second };
	}

	const bgfx::Memory* mem = nullptr;
	bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;

	if(nullptr != data && size > 0)
	{
		mem = bgfx::makeRef(data, size);
	}

	if(depth > 1)
	{
		texture = bgfx::createTexture3D(width, height, depth, false, format, flags, mem);
	}
	else
	{
		texture = bgfx::createTexture2D(width, height, false, 1, format, flags, mem);
	}

	if(bgfx::isValid(texture))
	{
		bgfx::setName(texture, pName);
		m_textureHandleCaches[textureNameCrc] = texture.idx;
	}
	else
	{
		CD_ENGINE_ERROR("Faild to create texture {0}!", pName);
	}

	return texture;
}

bgfx::TextureHandle RenderContext::UpdateTexture(const char* pName, uint16_t layer, uint8_t mip, uint16_t x, uint16_t y, uint16_t z, uint16_t width, uint16_t height, uint16_t depth, const void* data, uint32_t size)
{
	bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
	const bgfx::Memory* mem = nullptr;

	StringCrc textureNameCrc{ pName };
	auto itTextureCache = m_textureHandleCaches.find(textureNameCrc);
	if (itTextureCache == m_textureHandleCaches.end())
	{
		CD_ENGINE_WARN("Texture handle of {} can not find!", pName);
		return handle;
	}

	if (nullptr != data && size > 0)
	{
		mem = bgfx::makeRef(data, size);
	}

	handle = { itTextureCache->second };
	if (depth > 1)
	{
		bgfx::updateTexture3D(handle, mip, x, y, z, width, height, depth, mem);
	}
	else
	{
		bgfx::updateTexture2D(handle, layer, mip, x, y, width, height, mem);
	}

	return handle;
}

bgfx::UniformHandle RenderContext::CreateUniform(const char* pName, bgfx::UniformType::Enum uniformType, uint16_t number)
{
	StringCrc uniformNameCrc{ pName };
	auto itUniformCache = m_uniformHandleCaches.find(uniformNameCrc);
	if (itUniformCache != m_uniformHandleCaches.end())
	{
		return { itUniformCache->second };
	}

	bgfx::UniformHandle uniformHandle = bgfx::createUniform(pName, uniformType, number);
	if(bgfx::isValid(uniformHandle))
	{
		m_uniformHandleCaches[uniformNameCrc] = uniformHandle.idx;
	}

	return uniformHandle;
}

bgfx::VertexLayout RenderContext::CreateVertexLayout(StringCrc resourceCrc, const std::vector<cd::VertexAttributeLayout>& vertexAttributes)
{
	auto itVertexLayoutCache = m_vertexLayoutCaches.find(resourceCrc);
	if (itVertexLayoutCache != m_vertexLayoutCaches.end())
	{
		return itVertexLayoutCache->second;
	}

	bgfx::VertexLayout newVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(newVertexLayout, vertexAttributes);
	m_vertexLayoutCaches[resourceCrc] = newVertexLayout;
	return newVertexLayout;
}

bgfx::VertexLayout RenderContext::CreateVertexLayout(StringCrc resourceCrc, const cd::VertexAttributeLayout& vertexAttribute)
{
	auto itVertexLayoutCache = m_vertexLayoutCaches.find(resourceCrc);
	if (itVertexLayoutCache != m_vertexLayoutCaches.end())
	{
		return itVertexLayoutCache->second;
	}

	bgfx::VertexLayout newVertexLayout;
	VertexLayoutUtility::CreateVertexLayout(newVertexLayout, vertexAttribute);
	m_vertexLayoutCaches[resourceCrc] = newVertexLayout;
	return newVertexLayout;
}

void RenderContext::SetVertexLayout(StringCrc resourceCrc, bgfx::VertexLayout vertexLayoutHandle)
{
	m_vertexLayoutCaches[resourceCrc] = std::move(vertexLayoutHandle);
}

void RenderContext::SetTexture(StringCrc resourceCrc, bgfx::TextureHandle textureHandle)
{
	m_textureHandleCaches[resourceCrc] = textureHandle.idx;
}

void RenderContext::SetUniform(StringCrc resourceCrc, bgfx::UniformHandle uniformreHandle)
{
	m_uniformHandleCaches[resourceCrc] = uniformreHandle.idx;
}

void RenderContext::FillUniform(StringCrc resourceCrc, const void *pData, uint16_t vec4Count) const
{
	bgfx::setUniform(GetUniform(resourceCrc), pData, vec4Count);
}

RenderTarget* RenderContext::GetRenderTarget(StringCrc resourceCrc) const
{
	auto itResource = m_renderTargetCaches.find(resourceCrc);
	if (itResource != m_renderTargetCaches.end())
	{
		return itResource->second.get();
	}

	return nullptr;
}

const bgfx::VertexLayout& RenderContext::GetVertexAttributeLayouts(StringCrc resourceCrc) const
{
	auto itResource = m_vertexLayoutCaches.find(resourceCrc);
	if (itResource != m_vertexLayoutCaches.end())
	{
		return itResource->second;
	}

	static bgfx::VertexLayout dummy;
	return dummy;
}

bgfx::TextureHandle RenderContext::GetTexture(StringCrc resourceCrc) const
{
	auto itResource = m_textureHandleCaches.find(resourceCrc);
	if (itResource != m_textureHandleCaches.end())
	{
		return { itResource->second };
	}

	return bgfx::TextureHandle{ bgfx::kInvalidHandle };
}

bgfx::UniformHandle RenderContext::GetUniform(StringCrc resourceCrc) const
{
	auto itResource = m_uniformHandleCaches.find(resourceCrc);
	if (itResource != m_uniformHandleCaches.end())
	{
		return { itResource->second };
	}

	return bgfx::UniformHandle{ bgfx::kInvalidHandle };
}

void RenderContext::DestoryRenderTarget(StringCrc resourceCrc)
{
	m_renderTargetCaches.erase(resourceCrc);
}

void RenderContext::DestoryTexture(StringCrc resourceCrc)
{
	auto it = m_textureHandleCaches.find(resourceCrc);
	if (it != m_textureHandleCaches.end())
	{
		assert(bgfx::isValid(bgfx::TextureHandle{ it->second }));
		bgfx::destroy(bgfx::TextureHandle{ it->second });
		m_textureHandleCaches.erase(it);
	}
}

void RenderContext::DestoryUniform(StringCrc resourceCrc)
{
	auto it = m_uniformHandleCaches.find(resourceCrc);
	if (it != m_uniformHandleCaches.end())
	{
		assert(bgfx::isValid(bgfx::UniformHandle{ it->second }));
		bgfx::destroy(bgfx::UniformHandle{ it->second });
		m_uniformHandleCaches.erase(it);
	}
}

}