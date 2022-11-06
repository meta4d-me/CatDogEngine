#pragma once

#include "Core/StringCrc.h"

#include <bgfx/bgfx.h>

#include <inttypes.h>
#include <memory>
#include <unordered_map>

namespace engine
{

class Camera;
class GBuffer;
class SwapChain;

constexpr uint8_t MaxViewCount = 255;
constexpr uint8_t InvalidSwapChainID = 0xff;
constexpr uint8_t MaxSwapChainCount = 8;

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
	void ResizeFrameBuffers(uint16_t width, uint16_t height);
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	uint16_t CreateView();
	uint8_t CreateSwapChain(void* pWindowHandle, uint16_t width, uint16_t height);
	SwapChain* GetSwapChain(uint8_t swapChainID) const;

	void InitGBuffer(uint16_t width, uint16_t height);
	GBuffer* GetGBuffer() const;

	void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }
	Camera* GetCamera() const { return m_pCamera; }

	/////////////////////////////////////////////////////////////////////
	// Resource related apis
	/////////////////////////////////////////////////////////////////////
	[[nodiscard]] bgfx::ShaderHandle CreateShader(const char* filePath);
	[[nodiscard]] bgfx::ProgramHandle CreateProgram(const char* pName, bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh);
	[[nodiscard]] bgfx::TextureHandle CreateTexture(const char* filePath, uint64_t flags = 0UL);
	[[nodiscard]] bgfx::UniformHandle CreateUniform(const char* pName, bgfx::UniformType::Enum uniformType, uint16_t number = 1);

	bgfx::ShaderHandle GetShader(StringCrc resourceCrc) const;
	bgfx::ProgramHandle GetProgram(StringCrc resourceCrc) const;
	bgfx::TextureHandle GetTexture(StringCrc resourceCrc) const;
	bgfx::UniformHandle GetUniform(StringCrc resourceCrc) const;

private:
	uint8_t m_currentViewCount = 0;
	uint8_t m_currentSwapChainCount = 0;
	Camera* m_pCamera = nullptr;
	std::unique_ptr<SwapChain> m_pSwapChains[MaxSwapChainCount];
	std::unique_ptr<GBuffer> m_pGBuffer;

	std::unordered_map<size_t, bgfx::ShaderHandle> m_shaderHandleCaches;
	std::unordered_map<size_t, bgfx::ProgramHandle> m_programHandleCaches;
	std::unordered_map<size_t, bgfx::TextureHandle> m_textureHandleCaches;
	std::unordered_map<size_t, bgfx::UniformHandle> m_uniformHandleCaches;
};

}