#include "RenderContext.h"

#include "Log/Log.h"
#include "Path/Path.h"
#include "Renderer.h"
#include "Rendering/ShaderCollections.h"
#include "Rendering/ShaderType.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Resources/ResourceLoader.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bimg/decode.h>
#include <bx/allocator.h>

#include <cassert>
//#include <format>
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

std::tuple<std::string_view, std::string_view, std::string_view> IdentifyShaderTypes(const std::set<std::string>& shaders)
{
	assert(shaders.size() <= 2);
	std::tuple<std::string_view, std::string_view, std::string_view> shadersTuple = { "", "", "" };
	for (const auto& name : shaders)
	{
		engine::ShaderType type = engine::GetShaderType(name);
		if (engine::ShaderType::Vertex == type)
		{
			std::get<0>(shadersTuple) = name;
		}
		else if (engine::ShaderType::Fragment == type)
		{
			std::get<1>(shadersTuple) = name;

		}
		else if (engine::ShaderType::Compute == type)
		{
			std::get<2>(shadersTuple) = name;
		}
		else
		{
			CD_ENGINE_WARN("Unknown shader type of {0}!", name);
		}
	}
	return shadersTuple;
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
	for (auto it : m_shaderHandles)
	{
		bgfx::destroy(bgfx::ShaderHandle{ it.second });
	}

	for (auto it : m_shaderProgramHandles)
	{
		bgfx::destroy(bgfx::ProgramHandle{ it.second });
	}
	
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

void RenderContext::Submit(uint16_t viewID, const std::string& programName, const std::string& featureCombine)
{
	assert(bgfx::isValid(GetShaderProgramHandle(programName, featureCombine)));
	bgfx::submit(viewID, GetShaderProgramHandle(programName, featureCombine));
}

void RenderContext::Dispatch(uint16_t viewID, const std::string& programName, uint32_t numX, uint32_t numY, uint32_t numZ)
{
	assert(bgfx::isValid(GetShaderProgramHandle(programName)));
	bgfx::dispatch(viewID, GetShaderProgramHandle(programName), numX, numY, numZ);
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

void RenderContext::RegisterShaderProgram(StringCrc programNameCrc, std::initializer_list<std::string> names)
{
	m_pShaderCollections->RegisterShaderProgram(programNameCrc, cd::MoveTemp(names));
}

void RenderContext::AddShaderFeature(StringCrc programNameCrc, std::string combine)
{
	m_pShaderCollections->AddFeatureCombine(programNameCrc, cd::MoveTemp(combine));
}

bool RenderContext::CheckShaderProgram(const std::string& programName, const std::string& featuresCombine)
{
	assert(m_pShaderCollections->IsProgramValid(StringCrc(programName)));

	if (!bgfx::isValid(GetShaderProgramHandle(programName, featuresCombine)))
	{
		// Its only represents that we do not hold the shader program GPU handle, 
		// whether the shader is compiled or not is unknown.
		// The Combile Task will still be added to the queue and ResourceBuilder ensures that
		// there is no duplication of compilation behavior.
		AddShaderCompileTask(ShaderCompileInfo(programName, featuresCombine));
		m_pShaderCollections->AddFeatureCombine(StringCrc(programName), featuresCombine);
		return false;
	}
	return true;
}

void RenderContext::UploadShaderProgram(const std::string& programName, const std::string& featuresCombine)
{
	assert(m_pShaderCollections->IsProgramValid(StringCrc(programName)));
	
	auto [vsName, fsName, csName] = IdentifyShaderTypes(m_pShaderCollections->GetShaders(StringCrc(programName)));

	if (featuresCombine.empty())
	{
		// Non-uber shader case.
		if (!vsName.empty() && !fsName.empty() && csName.empty())
		{
			CreateProgram(programName, vsName.data(), fsName.data());
		}
		else if (!csName.empty())
		{
			CreateProgram(programName, csName.data());
		}
		else
		{
			CD_ENGINE_WARN("Unknown non-uber shader program type of {0}!", programName);
		}
	}
	else
	{
		// Uber shader case.
		if (!vsName.empty() && !fsName.empty() && csName.empty())
		{
			CreateProgram(programName, vsName.data(), fsName.data(), featuresCombine);
		}
		else
		{
			CD_ENGINE_WARN("Unknown uber shader program type of {0}!", programName);
		}
	}
}

void RenderContext::AddShaderCompileTask(ShaderCompileInfo info)
{
	const auto& it = std::find(m_shaderCompileTasks.begin(), m_shaderCompileTasks.end(), info);
	if (it == m_shaderCompileTasks.end())
	{
		CD_ENGINE_INFO("Shader compile task added for {0} with shader features : [{1}]", info.m_programName, info.m_featuresCombine);
		m_shaderCompileTasks.emplace_back(cd::MoveTemp(info));
	}
}

void RenderContext::ClearShaderCompileTasks()
{
	m_shaderCompileTasks.clear();
}

void RenderContext::SetShaderCompileTasks(std::vector<ShaderCompileInfo> tasks)
{
	m_shaderCompileTasks = cd::MoveTemp(tasks);
}

const RenderContext::ShaderBlob& RenderContext::AddShaderBlob(StringCrc shaderNameCrc, ShaderBlob blob)
{
	const auto& it = m_shaderBlobs.find(shaderNameCrc);
	if (it != m_shaderBlobs.end())
	{
		return *(it->second);
	}
	m_shaderBlobs[shaderNameCrc] = std::make_unique<ShaderBlob>(cd::MoveTemp(blob));
	return *m_shaderBlobs.at(shaderNameCrc);
}

const RenderContext::ShaderBlob& RenderContext::GetShaderBlob(StringCrc shaderNameCrc) const
{
	assert(m_shaderBlobs.find(shaderNameCrc) != m_shaderBlobs.end() && "Shader blob does not exist!");
	return *m_shaderBlobs.at(shaderNameCrc).get();
}

void RenderContext::SetShaderProgramHandle(const std::string& programName, bgfx::ProgramHandle handle, const std::string& featureCombine)
{
	m_shaderProgramHandles[StringCrc(programName + featureCombine)] = handle.idx;
}

bgfx::ProgramHandle RenderContext::GetShaderProgramHandle(const std::string& programName, const std::string& featureCombine) const
{
	const auto& it = m_shaderProgramHandles.find(StringCrc(programName + featureCombine));
	if (it != m_shaderProgramHandles.end())
	{
		return { it->second };
	}

	return { bgfx::kInvalidHandle };
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

bgfx::ShaderHandle RenderContext::CreateShader(const char* pShaderName)
{
	StringCrc shaderNameCrc(pShaderName);
	auto itShaderCache = m_shaderHandles.find(shaderNameCrc);
	if(itShaderCache != m_shaderHandles.end())
	{
		return { itShaderCache->second };
	}

	std::string shaderFileFullPath = Path::GetShaderOutputPath(pShaderName);
	const auto& shaderBlob = AddShaderBlob(shaderNameCrc, ResourceLoader::LoadFile(shaderFileFullPath.c_str()));
	bgfx::ShaderHandle shaderHandle = bgfx::createShader(bgfx::makeRef(shaderBlob.data(), static_cast<uint32_t>(shaderBlob.size())));

	if(bgfx::isValid(shaderHandle))
	{
		bgfx::setName(shaderHandle, pShaderName);
		m_shaderHandles[shaderNameCrc] = shaderHandle.idx;
	}

	return shaderHandle;
}

bgfx::ProgramHandle RenderContext::CreateProgram(const std::string& programName, const std::string& csName)
{
	StringCrc programNameCrc(programName);
	auto itProgram = m_shaderProgramHandles.find(programNameCrc);
	if (itProgram != m_shaderProgramHandles.end())
	{
		return { itProgram->second };
	}

	bgfx::ProgramHandle programHandle = bgfx::createProgram(CreateShader(csName.c_str()));
	if (bgfx::isValid(programHandle))
	{
		m_shaderProgramHandles[programNameCrc] = programHandle.idx;
	}

	return programHandle;
}

bgfx::ProgramHandle RenderContext::CreateProgram(const std::string& programName, const std::string& vsName, const std::string& fsName, const std::string& featureCombine)
{
	StringCrc fullProgramNameCrc = StringCrc(programName + featureCombine);

	const auto& it = m_shaderProgramHandles.find(fullProgramNameCrc);
	if (it != m_shaderProgramHandles.end())
	{
		return { it->second };
	}

	std::string inputVSFilePath = engine::Path::GetBuiltinShaderInputPath(vsName.c_str());
	std::string outputFSFilePath = engine::Path::GetShaderOutputPath(fsName.c_str(), featureCombine);

	bgfx::ShaderHandle vsHandle = CreateShader(inputVSFilePath.c_str());
	bgfx::ShaderHandle fsHandle = CreateShader(outputFSFilePath.c_str());
	bgfx::ProgramHandle programHandle = bgfx::createProgram(vsHandle, fsHandle);

	if (bgfx::isValid(programHandle))
	{
		m_shaderProgramHandles[fullProgramNameCrc] = programHandle.idx;
	}

	return programHandle;
}

bgfx::TextureHandle RenderContext::CreateTexture(const char* pFilePath, uint64_t flags)
{
	StringCrc filePathCrc(pFilePath);
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
		return bgfx::TextureHandle{bgfx::kInvalidHandle};
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

	bgfx::TextureHandle handle{bgfx::kInvalidHandle};
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
	StringCrc textureNameCrc(pName);
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

	StringCrc textureNameCrc(pName);
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
	StringCrc uniformNameCrc(pName);
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

uint16_t RenderContext::GetViewportViewID(void* pViewportViewID) const
{
	struct CastPtrToViewID
	{
		union
		{
			void* pUserData;
			uint16_t viewID;
		};
	};

	CastPtrToViewID cast;
	cast.pUserData = pViewportViewID;
	uint16_t viewID = cast.viewID;
	return viewID;
}

StringCrc RenderContext::GetRenderTargetCrc(void* pViewportViewID) const
{
	return StringCrc(std::format("ViewportRT_{}", GetViewportViewID(pViewportViewID)));
}

RenderTarget* RenderContext::GetRenderTarget(void* pViewportViewID) const
{
	return GetRenderTarget(GetRenderTargetCrc(pViewportViewID));
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

const bgfx::VertexLayout& RenderContext::GetVertexLayout(StringCrc resourceCrc) const
{
	auto itResource = m_vertexLayoutCaches.find(resourceCrc);
	if (itResource != m_vertexLayoutCaches.end())
	{
		return itResource->second;
	}

	static bgfx::VertexLayout dummy;
	return dummy;
}

bgfx::ShaderHandle RenderContext::GetShader(StringCrc resourceCrc) const
{
	auto itResource = m_shaderHandles.find(resourceCrc);
	if (itResource != m_shaderHandles.end())
	{
		return { itResource->second };
	}

	return bgfx::ShaderHandle{ bgfx::kInvalidHandle };
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

void RenderContext::DestoryShader(StringCrc resourceCrc)
{
	auto it = m_shaderHandles.find(resourceCrc);
	if (it != m_shaderHandles.end())
	{
		assert(bgfx::isValid(bgfx::ShaderHandle{ it->second }));
		bgfx::destroy(bgfx::ShaderHandle{ it->second });
		m_shaderHandles.erase(it);
	}
}

void RenderContext::DestoryProgram(StringCrc resourceCrc)
{
	auto it = m_shaderProgramHandles.find(resourceCrc);
	if (it != m_shaderProgramHandles.end())
	{
		assert(bgfx::isValid(bgfx::ProgramHandle{ it->second }));
		bgfx::destroy(bgfx::ProgramHandle{ it->second });
		m_shaderProgramHandles.erase(it);
	}
}

}