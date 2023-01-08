#pragma once

#include "Core/StringCrc.h"
#include "Math/Matrix.hpp"
#include "RenderTarget.h"
#include "Scene/VertexAttribute.h"
#include "Scene/VertexFormat.h"

#include <bgfx/bgfx.h>

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace engine
{

class Camera;
class Renderer;

static constexpr uint8_t MaxViewCount = 255;
static constexpr uint8_t MaxRenderTargetCount = 255;

// In current design, RenderContext needs to be a singleton.
// The reason is that it binds to bgfx graphics initialization which should only happen once.
class RenderContext
{
public:
	explicit RenderContext() = default;
	RenderContext(const RenderContext&) = delete;
	RenderContext& operator=(const RenderContext&) = delete;
	RenderContext(RenderContext&&) = delete;
	RenderContext& operator=(RenderContext&&) = delete;
	~RenderContext();

	void Init();
	void OnResize(uint16_t width, uint16_t height);
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	uint16_t CreateView();

	void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }
	Camera* GetCamera() const { return m_pCamera; }

	/////////////////////////////////////////////////////////////////////
	// Resource related apis
	/////////////////////////////////////////////////////////////////////
	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, std::vector<AttachmentDescriptor> attachmentDescs);
	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, uint16_t width, uint16_t height, void* pWindowHandle);
	RenderTarget* CreateRenderTarget(StringCrc resourceCrc, std::unique_ptr<RenderTarget> pRenderTarget);
	bgfx::ShaderHandle CreateShader(const char* filePath);
	bgfx::ProgramHandle CreateProgram(const char* pName, const char* pVSName, const char* pFSName);
	bgfx::ProgramHandle CreateProgram(const char* pName, bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh);
	bgfx::ProgramHandle CreateProgram(const char *pName, const char *pCSName);
	bgfx::ProgramHandle CreateProgram(const char *pName, bgfx::ShaderHandle csh);
	bgfx::TextureHandle CreateTexture(const char* filePath, uint64_t flags = 0UL);
	bgfx::TextureHandle CreateTexture(const char *pName, const uint16_t _width, const uint16_t _height, uint64_t flags = 0UL);
	bgfx::TextureHandle CreateTexture(const char *pName, const uint16_t _width, const uint16_t _height, const uint16_t _depth, uint64_t flags = 0UL);
	bgfx::UniformHandle CreateUniform(const char* pName, bgfx::UniformType::Enum uniformType, uint16_t number = 1);

	bgfx::VertexLayout CreateVertexLayout(StringCrc resourceCrc, const std::vector<cd::VertexAttributeLayout>& vertexAttributes);
	bgfx::VertexLayout CreateVertexLayout(StringCrc resourceCrc, const cd::VertexAttributeLayout& vertexAttribute);
	void SetVertexLayout(StringCrc resourceCrc, bgfx::VertexLayout textureHandle);
	void SetTexture(StringCrc resourceCrc, bgfx::TextureHandle textureHandle);
	void SetUniform(StringCrc resourceCrc, bgfx::UniformHandle uniformreHandle);

	void FillUniform(StringCrc resourceCrc, const void *pData, uint16_t num = static_cast<uint16_t>(1U)) const;

	RenderTarget* GetRenderTarget(StringCrc resourceCrc) const;
	const bgfx::VertexLayout& GetVertexLayout(StringCrc resourceCrc) const;
	const bgfx::ShaderHandle& GetShader(StringCrc resourceCrc) const;
	const bgfx::ProgramHandle& GetProgram(StringCrc resourceCrc) const;
	const bgfx::TextureHandle& GetTexture(StringCrc resourceCrc) const;
	const bgfx::UniformHandle& GetUniform(StringCrc resourceCrc) const;

private:
	uint8_t m_currentViewCount = 0;
	Camera* m_pCamera = nullptr;
	std::unordered_map<size_t, std::unique_ptr<RenderTarget>> m_renderTargetCaches;
	std::unordered_map<size_t, bgfx::VertexLayout> m_vertexLayoutCaches;
	std::unordered_map<size_t, bgfx::ShaderHandle> m_shaderHandleCaches;
	std::unordered_map<size_t, bgfx::ProgramHandle> m_programHandleCaches;
	std::unordered_map<size_t, bgfx::TextureHandle> m_textureHandleCaches;
	std::unordered_map<size_t, bgfx::UniformHandle> m_uniformHandleCaches;
};

}