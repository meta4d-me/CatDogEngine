#pragma once

#include "Core/StringCrc.h"
#include "Graphics/GraphicsBackend.h"
#include "Math/Matrix.hpp"
#include "Rendering/ShaderCompileInfo.h"
#include "RenderTarget.h"
#include "Scene/VertexAttribute.h"
#include "Scene/VertexFormat.h"

#include "bgfx/bgfx.h"

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
class ShaderCollections;

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
	void Submit(uint16_t viewID, const std::string& programName, const std::string& featuresCombine = "");
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
	// Shader collections apis
	/////////////////////////////////////////////////////////////////////
	void SetShaderCollections(ShaderCollections* pShaderCollections) { m_pShaderCollections = pShaderCollections; }
	const ShaderCollections* GetShaderCollections() const { return m_pShaderCollections; }

	void RegisterShaderProgram(StringCrc programNameCrc, std::initializer_list<std::string> names);
	void AddShaderFeature(StringCrc programNameCrc, std::string combine);

	bool CheckShaderProgram(Entity entity, const std::string& programName, const std::string& featuresCombine = "");
	bool OnShaderHotModified(Entity entity, const std::string& programName, const std::string& featuresCombine = "");
	void UploadShaderProgram(const std::string& programName, const std::string& featuresCombine = "");
	void DestroyShaderProgram(const std::string& programName, const std::string& featuresCombine = "");

	void AddShaderCompileInfo(ShaderCompileInfo info);
	void ClearShaderCompileInfos();
	void SetShaderCompileInfos(std::set<ShaderCompileInfo> tasks);
	std::set<ShaderCompileInfo>& GetShaderCompileInfos() { return m_shaderCompileInfos; }
	const std::set<ShaderCompileInfo>& GetShaderCompileInfos() const { return m_shaderCompileInfos; }

	void CheckModifiedProgram(std::string modifiedShaderName);
	void ClearModifiedProgramNameCrcs();
	std::set<StringCrc>& GetModifiedProgramNameCrcs() { return m_modifiedProgramNameCrcs; }
	const std::set<StringCrc>& GetModifiedProgramNameCrcs() const { return m_modifiedProgramNameCrcs; }

	/////////////////////////////////////////////////////////////////////
	// Shader blob apis
	/////////////////////////////////////////////////////////////////////
	const RenderContext::ShaderBlob& AddShaderBlob(StringCrc shaderNameCrc, ShaderBlob blob);
	const ShaderBlob& GetShaderBlob(StringCrc shaderNameCrc) const;

	/////////////////////////////////////////////////////////////////////
	// Resource related apis
	/////////////////////////////////////////////////////////////////////

	bool IsShaderProgramValid(const std::string& programName, const std::string& featuresCombine = "") const;

	void SetShaderProgramHandle(const std::string& programName, bgfx::ProgramHandle handle, const std::string& featuresCombine = "");
	bgfx::ProgramHandle GetShaderProgramHandle(const std::string& programName, const std::string& featuresCombine = "") const;

	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, std::vector<AttachmentDescriptor> attachmentDescs);
	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, void* pWindowHandle);
	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, std::unique_ptr<RenderTarget> pRenderTarget);

	bgfx::ShaderHandle CreateShader(const char* filePath, const std::string& combine = "");
	bgfx::ProgramHandle CreateProgram(const std::string& programName, const std::string& csName);
	bgfx::ProgramHandle CreateProgram(const std::string& programName, const std::string& vsName, const std::string& fsName, const std::string& featuresCombine = "");

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
	uint16_t m_backBufferWidth;
	uint16_t m_backBufferHeight;

	std::unordered_map<StringCrc, std::unique_ptr<RenderTarget>> m_renderTargetCaches;
	std::unordered_map<StringCrc, bgfx::VertexLayout> m_vertexLayoutCaches;
	std::unordered_map<StringCrc, uint16_t> m_textureHandleCaches;
	std::unordered_map<StringCrc, uint16_t> m_uniformHandleCaches;

	ShaderCollections* m_pShaderCollections = nullptr;

	// Key : StringCrc(Program name), Value : Shader program handle
	std::unordered_map<StringCrc, uint16_t> m_shaderProgramHandles;

	// Key : StringCrc(Shader name), Value : Shader handle
	std::unordered_map<StringCrc, uint16_t> m_shaderHandles;
	// Key : StringCrc(Shader name), Value : Shader binary data
	std::unordered_map<StringCrc, std::unique_ptr<ShaderBlob>> m_shaderBlobs;

	std::set<ShaderCompileInfo> m_shaderCompileInfos;
	std::set<StringCrc> m_modifiedProgramNameCrcs;
};

}