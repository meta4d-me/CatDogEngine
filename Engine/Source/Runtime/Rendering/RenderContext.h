#pragma once

#include "Core/StringCrc.h"
#include "Graphics/GraphicsBackend.h"
#include "Math/Matrix.hpp"
#include "RenderTarget.h"
#include "Scene/VertexAttribute.h"
#include "Scene/VertexFormat.h"

#include <bgfx/bgfx.h>

#include <cstdint>
#include <map>
#include <memory>
#include <tuple>
#include <unordered_map>

namespace engine
{

class Camera;
class Renderer;
class ShaderVariantCollections;

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
	void Submit(uint16_t viewID, const std::string& programName);
	void Submit(uint16_t viewID, const std::string& programName, const StringCrc featureCombineCrc);
	void Dispatch(uint16_t viewID, const std::string& programName, uint32_t numX, uint32_t numY, uint32_t numZ);
	void EndFrame();
	void Shutdown();

	uint16_t GetBackBufferWidth() const { return m_backBufferWidth; }
	uint16_t GetBackBufferHeight() const { return m_backBufferHeight; }
	void SetBackBufferSize(uint16_t width, uint16_t height) { m_backBufferWidth = width; m_backBufferHeight = height; }

	uint16_t CreateView();
	void ResetViewCount() { m_currentViewCount = 0; }
	uint16_t GetCurrentViewCount() const { return m_currentViewCount; }

	/////////////////////////////////////////////////////////////////////
	// Shader variant collections apis
	/////////////////////////////////////////////////////////////////////
	void SetShaderVariantCollections(ShaderVariantCollections* pShaderVariantCollections) { m_pShaderVariantCollections = pShaderVariantCollections; }
	ShaderVariantCollections* GetShaderVariantCollections() { return m_pShaderVariantCollections; }

	void RegisterNonUberShader(std::string programName, std::initializer_list<std::string> names);
	void RegisterUberShader(std::string programName, std::initializer_list<std::string> names, std::initializer_list<std::string> combines = {});

	void UploadShaders(std::string programName);

	/////////////////////////////////////////////////////////////////////
	// Shader blob apis
	/////////////////////////////////////////////////////////////////////

	const RenderContext::ShaderBlob& AddShaderBlob(StringCrc shaderNameCrc, ShaderBlob blob);
	const ShaderBlob& GetShaderBlob(StringCrc shaderNameCrc) const;

	/////////////////////////////////////////////////////////////////////
	// Resource related apis
	/////////////////////////////////////////////////////////////////////
	void SetNonUberShaderProgramHandle(const StringCrc programNameCrc, bgfx::ProgramHandle handle) { m_nonUberShaderProgramHandles[programNameCrc.Value()] = handle.idx; }
	bgfx::ProgramHandle GetNonUberShaderProgramHandle(const StringCrc programName) const;

	void SetUberShaderProgramHandle(const StringCrc programName, const StringCrc featureCombineCrc, bgfx::ProgramHandle handle) { m_uberShaderProgramHandles[programName.Value()][featureCombineCrc.Value()] = handle.idx; }
	bgfx::ProgramHandle GetUberShaderProgramHandle(const StringCrc programName, const StringCrc featureCombineCrc) const;

	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, std::vector<AttachmentDescriptor> attachmentDescs);
	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, void* pWindowHandle);
	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, std::unique_ptr<RenderTarget> pRenderTarget);

	bgfx::ShaderHandle CreateShader(const char* filePath);
	bgfx::ProgramHandle CreateProgram(const char* pName, const char* pCSName);
	bgfx::ProgramHandle CreateProgram(const char* pName, const char* pVSName, const char* pFSName);
	bgfx::ProgramHandle CreateProgram(const char* pName, const char* pVSName, const char* pFSName, const char* pFeatureCombine);
	bgfx::ProgramHandle CreateProgram(const char* pName, bgfx::ShaderHandle csh);
	bgfx::ProgramHandle CreateProgram(const char* pName, bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh);

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
	const bgfx::VertexLayout& GetVertexLayout(StringCrc resourceCrc) const;
	bgfx::ShaderHandle GetShader(StringCrc resourceCrc) const;
	bgfx::TextureHandle GetTexture(StringCrc resourceCrc) const;
	bgfx::UniformHandle GetUniform(StringCrc resourceCrc) const;

	void DestoryRenderTarget(StringCrc resourceCrc);
	void DestoryTexture(StringCrc resourceCrc);
	void DestoryUniform(StringCrc resourceCrc);
	void DestoryShader(StringCrc resourceCrc);
	void DestoryProgram(StringCrc resourceCrc);

private:
	uint8_t m_currentViewCount = 0;
	std::unordered_map<size_t, std::unique_ptr<RenderTarget>> m_renderTargetCaches;
	std::unordered_map<size_t, bgfx::VertexLayout> m_vertexLayoutCaches;
	std::unordered_map<size_t, bgfx::TextureHandle> m_textureHandleCaches;
	std::unordered_map<size_t, bgfx::UniformHandle> m_uniformHandleCaches;

	ShaderVariantCollections* m_pShaderVariantCollections = nullptr;

	// Key : StringCrc(Program name), Value : Non-uber hader shader program handle
	std::map<uint32_t, uint16_t> m_nonUberShaderProgramHandles;
	// Key : StringCrc(Program name), Value : { Key : StringCrc(Feature combine), Value : Uber hader shader program handle }
	std::map<uint32_t, std::map<uint32_t, uint16_t>> m_uberShaderProgramHandles;

	// Key : StringCrc(Shader name), Value : Shader handle
	std::map<uint32_t, uint16_t> m_shaderHandles;
	// Key : StringCrc(Shader name), Value : Shader binary data
	std::map<uint32_t, std::unique_ptr<ShaderBlob>> m_shaderBlobs;

	uint16_t m_backBufferWidth;
	uint16_t m_backBufferHeight;
};

}