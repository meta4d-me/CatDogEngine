#pragma once

#include "Core/StringCrc.h"
#include "Graphics/GraphicsBackend.h"
#include "Math/Matrix.hpp"
#include "Rendering/ShaderType.h"
#include "RenderTarget.h"
#include "Scene/VertexAttribute.h"
#include "Scene/VertexFormat.h"

#include <bgfx/bgfx.h>

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <unordered_map>

namespace engine
{

class Camera;
class Renderer;
class ResourceContext;
class ShaderResource;

static constexpr uint8_t MaxViewCount = 255;
static constexpr uint8_t MaxRenderTargetCount = 255;

// In current design, RenderContext needs to be a singleton.
// The reason is that it binds to bgfx graphics initialization which should only happen once.
class RenderContext
{
public:
	using ShaderBlob = std::vector<std::byte>;

public:
	RenderContext() = default;
	RenderContext(const RenderContext&) = delete;
	RenderContext& operator=(const RenderContext&) = delete;
	RenderContext(RenderContext&&) = delete;
	RenderContext& operator=(RenderContext&&) = delete;
	~RenderContext();

	void Init(GraphicsBackend backend, void* hwnd = nullptr);
	void OnResize(uint16_t width, uint16_t height);
	void BeginFrame();
	void Submit(uint16_t viewID, uint16_t programHandle);
	void Submit(uint16_t viewID, StringCrc programHandleIndex);
	void Dispatch(uint16_t viewID, uint16_t programHandle, uint32_t numX, uint32_t numY, uint32_t numZ);
	void Dispatch(uint16_t viewID, StringCrc programHandleIndex, uint32_t numX, uint32_t numY, uint32_t numZ);
	void EndFrame();
	void Shutdown();

	void SetResourceContext(ResourceContext* pContext) { m_pResourceContext = pContext; }
	ResourceContext* GetResourceContext() const { return m_pResourceContext; }

	uint16_t GetBackBufferWidth() const { return m_backBufferWidth; }
	uint16_t GetBackBufferHeight() const { return m_backBufferHeight; }
	void SetBackBufferSize(uint16_t width, uint16_t height) { m_backBufferWidth = width; m_backBufferHeight = height; }

	uint16_t CreateView();
	void ResetViewCount() { m_currentViewCount = 0; }
	uint16_t GetCurrentViewCount() const { return m_currentViewCount; }

	// For Standard ShaderProgramType
	ShaderResource* RegisterShaderProgram(const std::string& programName, const std::string& vsName, const std::string& fsName, const std::string& combine = "");
	// For non-Standard ShaderProgramType
	ShaderResource* RegisterShaderProgram(const std::string& programName, const std::string& shaderName, ShaderProgramType type, const std::string& combine = "");

	void AddShaderResource(StringCrc shaderName, ShaderResource* resource) { m_shaderResources.insert({ shaderName, resource }); }
	void DeleteShaderResource(StringCrc shaderName) { m_shaderResources.erase(shaderName); }
	void SetShaderResources(std::multimap<StringCrc, ShaderResource*> resources) { m_shaderResources = cd::MoveTemp(resources); }
	std::multimap<StringCrc, ShaderResource*>& GetShaderResources() { return m_shaderResources; }
	const std::multimap<StringCrc, ShaderResource*>& GetShaderResources() const { return m_shaderResources; }

	// Call back function bind to file watcher.
	void OnShaderHotModified(std::string modifiedShaderName);
	void ClearModifiedShaderResources() { m_modifiedShaderResources.clear(); }
	std::set<ShaderResource*>& GetModifiedShaderResources() { return m_modifiedShaderResources; }
	const std::set<ShaderResource*>& GetModifiedShaderResources() const { return m_modifiedShaderResources; }

	void OnShaderRecompile();
	void AddRecompileShaderResource(ShaderResource* pShaderResource) { m_recompileShaderResources.insert(pShaderResource); }
	void DeleteRecompileShaderResource(ShaderResource* pShaderResource) { m_recompileShaderResources.erase(pShaderResource); }
	void ClearRecompileShaderResources() { m_recompileShaderResources.clear(); }
	void SetRecompileShaderResources(std::set<ShaderResource*> recompileShaderResources) { m_recompileShaderResources = cd::MoveTemp(recompileShaderResources); }
	std::set<ShaderResource*> GetRecompileShaderResources() { return m_recompileShaderResources; }
	const std::set<ShaderResource*> GetRecompileShaderResources() const { return m_recompileShaderResources; }

	void AddCompileFailedEntity(uint32_t entity);
	void ClearCompileFailedEntity();
	std::set<uint32_t>& GetCompileFailedEntities() { return m_compileFailedEntities; }
	const std::set<uint32_t>& GetCompileFailedEntities() const { return m_compileFailedEntities; }

	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, std::vector<AttachmentDescriptor> attachmentDescs);
	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, void* pWindowHandle);
	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, std::unique_ptr<RenderTarget> pRenderTarget);

	bgfx::TextureHandle CreateTexture(const char* filePath, uint64_t flags = 0UL);
	bgfx::TextureHandle CreateTexture(const char* pName, uint16_t width, uint16_t height, uint16_t depth, bgfx::TextureFormat::Enum format, uint64_t flags = 0UL, const void* data = nullptr, uint32_t size = 0);
	bgfx::TextureHandle UpdateTexture(const char* pName, uint16_t layer, uint8_t mip, uint16_t x, uint16_t y, uint16_t z, uint16_t width, uint16_t height, uint16_t depth, const void* data = nullptr, uint32_t size = 0);
	
	bgfx::UniformHandle CreateUniform(const char* pName, bgfx::UniformType::Enum uniformType, uint16_t number = 1);

	bgfx::VertexLayout CreateVertexLayout(StringCrc resourceCrc, const std::vector<cd::VertexAttributeLayout>& vertexAttributes);
	bgfx::VertexLayout CreateVertexLayout(StringCrc resourceCrc, const cd::VertexAttributeLayout& vertexAttribute);
	void SetVertexLayout(StringCrc resourceCrc, bgfx::VertexLayout textureHandle);
	void SetTexture(StringCrc resourceCrc, bgfx::TextureHandle textureHandle);
	void SetUniform(StringCrc resourceCrc, bgfx::UniformHandle uniformreHandle);
	void FillUniform(StringCrc resourceCrc, const void *pData, uint16_t vec4Count = 1) const;

	RenderTarget* GetRenderTarget(StringCrc resourceCrc) const;
	const bgfx::VertexLayout& GetVertexAttributeLayouts(StringCrc resourceCrc) const;
	bgfx::TextureHandle GetTexture(StringCrc resourceCrc) const;
	bgfx::UniformHandle GetUniform(StringCrc resourceCrc) const;

	void DestoryRenderTarget(StringCrc resourceCrc);
	void DestoryTexture(StringCrc resourceCrc);
	void DestoryUniform(StringCrc resourceCrc);

private:
	ResourceContext* m_pResourceContext = nullptr;

	uint8_t m_currentViewCount = 0;
	uint16_t m_backBufferWidth;
	uint16_t m_backBufferHeight;

	std::unordered_map<StringCrc, std::unique_ptr<RenderTarget>> m_renderTargetCaches;
	std::unordered_map<StringCrc, bgfx::VertexLayout> m_vertexLayoutCaches;
	std::unordered_map<StringCrc, uint16_t> m_textureHandleCaches;
	std::unordered_map<StringCrc, uint16_t> m_uniformHandleCaches;

	// Key : StringCrc(shader name), Value : ShaderResource*
	std::multimap<StringCrc, ShaderResource*> m_shaderResources;
	std::set<ShaderResource*> m_modifiedShaderResources;
	std::set<ShaderResource*> m_recompileShaderResources;
	std::set<uint32_t> m_compileFailedEntities;
};

}