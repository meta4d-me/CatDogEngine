#pragma once

#include <inttypes.h>
#include <memory>

namespace engine
{

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
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	uint16_t CreateView();
	uint8_t CreateSwapChain(void* pWindowHandle, uint16_t width, uint16_t height);
	SwapChain* GetSwapChain(uint8_t swapChainID) const;

	void InitGBuffer(uint16_t width, uint16_t height);
	GBuffer* GetGBuffer() const;

private:
	uint8_t m_currentViewCount = 0;
	uint8_t m_currentSwapChainCount = 0;
	std::unique_ptr<SwapChain> m_pSwapChains[MaxSwapChainCount];

	std::unique_ptr<GBuffer> m_pGBuffer;
};

}