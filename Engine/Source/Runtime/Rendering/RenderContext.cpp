#include "RenderContext.h"

#include "Log/Log.h"
#include "Path/Path.h"
#include "Renderer.h"
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
	for (auto it : m_nonUberShaderProgramHandles)
	{
		bgfx::destroy(bgfx::ProgramHandle{ it.second });
	}

	for (auto program : m_uberShaderProgramHandles)
	{
		for (auto variant : program.second)
		{
			bgfx::destroy(bgfx::ProgramHandle{ variant.second });
		}
	}

	for(auto it : m_shaderHandles)
	{
		bgfx::destroy(bgfx::ProgramHandle{ it.second });
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

void RenderContext::Submit(uint16_t viewID, const std::string& programName)
{
	if (m_shaderVariantCollections.IsNonUberShaderProgramValid(programName))
	{
		bgfx::ProgramHandle handle = GetNonUberShaderProgramHandle(StringCrc(programName));
		assert(bgfx::isValid(handle));
		bgfx::submit(viewID, handle);
	}
	else
	{
		CD_ENGINE_WARN("Unregistered non-uber program {0}!", programName);
	}
}

void RenderContext::Submit(uint16_t viewID, const std::string& programName, StringCrc featuresCombineCrc)
{
	if (m_shaderVariantCollections.IsUberShaderProgramValid(programName))
	{
		bgfx::ProgramHandle handle = GetUberShaderProgramHandle(StringCrc(programName), featuresCombineCrc);
		assert(bgfx::isValid(handle));
		bgfx::submit(viewID, handle);
	}
	else
	{
		CD_ENGINE_WARN("Unregistered uber program {0}!", programName);
	}
}

void RenderContext::Dispatch(uint16_t viewID, const std::string& programName, uint32_t numX, uint32_t numY, uint32_t numZ)
{
	bgfx::dispatch(viewID, GetNonUberShaderProgramHandle(StringCrc(programName)), numX, numY, numZ);
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

void RenderContext::RegisterNonUberShader(std::string programName, std::initializer_list<std::string> names)
{
	m_shaderVariantCollections.RegisterNonUberShader(cd::MoveTemp(programName), cd::MoveTemp(names));
}

void RenderContext::RegisterUberShader(std::string programName, std::initializer_list<std::string> names, std::initializer_list<std::string> combines)
{
	m_shaderVariantCollections.RegisterUberShader(cd::MoveTemp(programName), cd::MoveTemp(names), cd::MoveTemp(combines));
}

bool RenderContext::CheckNonUbeShaderProgram(const std::string& programName)
{
	assert(m_shaderVariantCollections.IsNonUberShaderProgramValid(programName));
	bgfx::ProgramHandle handle = GetNonUberShaderProgramHandle(StringCrc(programName));

	if (!bgfx::isValid(handle))
	{
		// Here only represents that we do not hold the shader GPU handle, whether the shader is compiled or not is unknown.
		// The Combile Task is still added to the queue, and ResourceBuilder ensures that,
		// there is no duplication of compilation behavior.
		AddShaderCompileTask(ShaderCompileInfo(programName));
		return false;
	}
	return true;
}

bool RenderContext::CheckUbeShaderProgram(const std::string& programName, const std::string& featuresCombine)
{
	assert(m_shaderVariantCollections.IsUberShaderProgramValid(programName));
	bgfx::ProgramHandle handle = GetUberShaderProgramHandle(StringCrc(programName), StringCrc(featuresCombine));

	if (!bgfx::isValid(handle))
	{
		AddShaderCompileTask(ShaderCompileInfo(programName, featuresCombine));
		if (!featuresCombine.empty())
		{
			m_shaderVariantCollections.AddFeatureCombine(programName, featuresCombine);
		}
		return false;
	}
	return true;
}

void RenderContext::UploadNonUberShader(const std::string& programName)
{
	assert(m_shaderVariantCollections.IsNonUberShaderProgramValid(programName));

	auto [vsName, fsName, csName] = IdentifyShaderTypes(m_shaderVariantCollections.GetNonUberShaders(programName));

	if (!vsName.empty() && !fsName.empty() && csName.empty())
	{
		CreateProgram(programName.c_str(), vsName.data(), fsName.data());
	}
	else if (!csName.empty())
	{
		CreateProgram(programName.c_str(), csName.data());
	}
	else
	{
		CD_ENGINE_WARN("Unknown non-uber shader program type of {0}!", programName);
	}
}

void RenderContext::UploadUberShader(const std::string& programName, const std::string& combine)
{
	assert(m_shaderVariantCollections.IsUberShaderProgramValid(programName));

	auto [vsName, fsName, csName] = IdentifyShaderTypes(m_shaderVariantCollections.GetUberShaders(programName));

	if (!vsName.empty() && !fsName.empty() && csName.empty())
	{
		CreateProgram(programName.c_str(), vsName.data(), fsName.data(), combine.c_str());
	}
	else
	{
		CD_ENGINE_WARN("Unknown uber shader program type of {0}!", programName);
	}
}

void RenderContext::AddShaderCompileTask(ShaderCompileInfo info)
{
	const auto& it = std::find(m_shaderCompileTasks.begin(), m_shaderCompileTasks.end(), info);
	if (it == m_shaderCompileTasks.end())
	{
		CD_ENGINE_INFO("Runtime shader compile task added for {0} with shader features : [ {1} ]", info.m_programName, info.m_featuresCombine);
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
	assert(m_shaderBlobs.find(shaderNameCrc.Value()) == m_shaderBlobs.end() && "Shader blob already exists!");
	m_shaderBlobs[shaderNameCrc.Value()] = std::make_unique<ShaderBlob>(cd::MoveTemp(blob));
	return *m_shaderBlobs.at(shaderNameCrc.Value());
}

const RenderContext::ShaderBlob& RenderContext::GetShaderBlob(StringCrc shaderNameCrc) const
{
	assert(m_shaderBlobs.find(shaderNameCrc.Value()) != m_shaderBlobs.end() && "Shader blob does not exist!");
	return *m_shaderBlobs.at(shaderNameCrc.Value()).get();
}

bgfx::ProgramHandle RenderContext::GetNonUberShaderProgramHandle(const StringCrc programName) const
{
	const auto& it = m_nonUberShaderProgramHandles.find(programName.Value());
	if (it != m_nonUberShaderProgramHandles.end())
	{
		return { it->second };
	}

	return { bgfx::kInvalidHandle };
}

bgfx::ProgramHandle RenderContext::GetUberShaderProgramHandle(const StringCrc programName, const StringCrc featureCombineCrc) const
{
	const auto& itProgram = m_uberShaderProgramHandles.find(programName.Value());
	if (itProgram == m_uberShaderProgramHandles.end())
	{
		return { bgfx::kInvalidHandle };
	}

	const auto& featureCombineProgramHandleMap = itProgram->second;
	const auto& itHandle = featureCombineProgramHandleMap.find(featureCombineCrc.Value());
	if (itHandle == featureCombineProgramHandleMap.end())
	{
		return { bgfx::kInvalidHandle };
	}

	return { itHandle->second };
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

bgfx::ShaderHandle RenderContext::CreateShader(const char* pShaderName)
{
	StringCrc shaderNameCrc(pShaderName);
	auto itShaderCache = m_shaderHandles.find(shaderNameCrc.Value());
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
		m_shaderHandles[shaderNameCrc.Value()] = shaderHandle.idx;
	}

	return shaderHandle;
}

bgfx::ProgramHandle RenderContext::CreateProgram(const char* pName, const char* pCSName)
{
	return CreateProgram(pName, CreateShader(pCSName));
}

bgfx::ProgramHandle RenderContext::CreateProgram(const char* pName, const char* pVSName, const char* pFSName)
{
	return CreateProgram(pName, CreateShader(pVSName), CreateShader(pFSName));
}

bgfx::ProgramHandle RenderContext::CreateProgram(const char* pName, const char* pVSName, const char* pFSName, const char* pFeatureCombine)
{
	// Uber shader program

	StringCrc programNameCrc(pName);
	StringCrc featureCombineCrc(pFeatureCombine);

	auto itProgram = m_uberShaderProgramHandles.find(programNameCrc.Value());
	if (itProgram != m_uberShaderProgramHandles.end())
	{
		const auto& variantHandleMap = (*itProgram).second;
		auto itHandle = variantHandleMap.find(featureCombineCrc.Value());
		if (itHandle != variantHandleMap.end())
		{
			return { variantHandleMap.at(featureCombineCrc.Value()) };
		}
	}

	std::string outputFSFilePath = engine::Path::GetShaderOutputPath(pFSName, pFeatureCombine);
	bgfx::ShaderHandle vsHandle = CreateShader(pVSName);
	bgfx::ShaderHandle fsHandle = CreateShader(outputFSFilePath.c_str());
	bgfx::ProgramHandle programHandle = bgfx::createProgram(vsHandle, fsHandle);

	if (bgfx::isValid(programHandle))
	{
		m_uberShaderProgramHandles[programNameCrc.Value()][featureCombineCrc.Value()] = programHandle.idx;
	}

	return programHandle;
}

bgfx::ProgramHandle RenderContext::CreateProgram(const char* pName, bgfx::ShaderHandle csh)
{
	StringCrc programNameCrc(pName);
	auto itProgram = m_nonUberShaderProgramHandles.find(programNameCrc.Value());
	if (itProgram != m_nonUberShaderProgramHandles.end())
	{
		return { itProgram->second };
	}

	bgfx::ProgramHandle program = bgfx::createProgram(csh);
	if (bgfx::isValid(program))
	{
		m_nonUberShaderProgramHandles[programNameCrc.Value()] = program.idx;
	}

	return program;
}

bgfx::ProgramHandle RenderContext::CreateProgram(const char* pName, bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh)
{
	StringCrc programNameCrc(pName);
	auto itProgram = m_nonUberShaderProgramHandles.find(programNameCrc.Value());
	if (itProgram != m_nonUberShaderProgramHandles.end())
	{
		return { itProgram->second };
	}

	bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh);
	if(bgfx::isValid(program))
	{
		m_nonUberShaderProgramHandles[programNameCrc.Value()] = program.idx;
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
		m_textureHandleCaches[filePath.Value()] = handle;
	}

	return handle;
}

bgfx::TextureHandle RenderContext::CreateTexture(const char* pName, uint16_t width, uint16_t height, uint16_t depth, bgfx::TextureFormat::Enum format, uint64_t flags, const void* data, uint32_t size)
{
	StringCrc textureName(pName);
	auto itTextureCache = m_textureHandleCaches.find(textureName.Value());
	if(itTextureCache != m_textureHandleCaches.end())
	{
		return itTextureCache->second;
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
		m_textureHandleCaches[textureName.Value()] = texture;
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

	StringCrc textureName(pName);
	auto itTextureCache = m_textureHandleCaches.find(textureName.Value());
	if (itTextureCache == m_textureHandleCaches.end())
	{
		CD_ENGINE_WARN("Texture handle of {} can not find!", pName);
		return handle;
	}

	if (nullptr != data && size > 0)
	{
		mem = bgfx::makeRef(data, size);
	}

	handle = itTextureCache->second;
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
	auto itResource = m_shaderHandles.find(resourceCrc.Value());
	if (itResource != m_shaderHandles.end())
	{
		return { itResource->second };
	}

	return bgfx::ShaderHandle{ bgfx::kInvalidHandle };
}

bgfx::TextureHandle RenderContext::GetTexture(StringCrc resourceCrc) const
{
	auto itResource = m_textureHandleCaches.find(resourceCrc.Value());
	if (itResource != m_textureHandleCaches.end())
	{
		return itResource->second;
	}

	return bgfx::TextureHandle{ bgfx::kInvalidHandle };
}

bgfx::UniformHandle RenderContext::GetUniform(StringCrc resourceCrc) const
{
	auto itResource = m_uniformHandleCaches.find(resourceCrc.Value());
	if (itResource != m_uniformHandleCaches.end())
	{
		return itResource->second;
	}

	return bgfx::UniformHandle{ bgfx::kInvalidHandle };
}

void RenderContext::DestoryRenderTarget(StringCrc resourceCrc)
{
	m_renderTargetCaches.erase(resourceCrc.Value());
}

void RenderContext::DestoryTexture(StringCrc resourceCrc)
{
	auto it = m_textureHandleCaches.find(resourceCrc.Value());
	if (it != m_textureHandleCaches.end())
	{
		assert(bgfx::isValid(it->second));
		bgfx::destroy(it->second);
		m_textureHandleCaches.erase(it);
	}
}

void RenderContext::DestoryUniform(StringCrc resourceCrc)
{
	auto it = m_uniformHandleCaches.find(resourceCrc.Value());
	if (it != m_uniformHandleCaches.end())
	{
		assert(bgfx::isValid(it->second));
		bgfx::destroy(it->second);
		m_uniformHandleCaches.erase(it);
	}
}

void RenderContext::DestoryShader(StringCrc resourceCrc)
{
	auto it = m_shaderHandles.find(resourceCrc.Value());
	if (it != m_shaderHandles.end())
	{
		assert(bgfx::isValid(bgfx::ShaderHandle{ it->second }));
		bgfx::destroy(bgfx::ShaderHandle{ it->second });
		m_shaderHandles.erase(it);
	}
}

void RenderContext::DestoryProgram(StringCrc resourceCrc)
{
	auto itNonuber = m_nonUberShaderProgramHandles.find(resourceCrc.Value());
	if (itNonuber != m_nonUberShaderProgramHandles.end())
	{
		assert(bgfx::isValid(bgfx::ProgramHandle{ itNonuber->second }));
		bgfx::destroy(bgfx::ProgramHandle{ itNonuber->second });
		m_nonUberShaderProgramHandles.erase(itNonuber);
	}

	auto itUber = m_uberShaderProgramHandles.find(resourceCrc.Value());
	if (itUber != m_uberShaderProgramHandles.end())
	{
		for (const auto& variant : itUber->second)
		{
			assert(bgfx::isValid(bgfx::ProgramHandle{ variant.second }));
			bgfx::destroy(bgfx::ProgramHandle{ variant.second });
		}
		m_uberShaderProgramHandles.erase(itUber);
	}
}

}