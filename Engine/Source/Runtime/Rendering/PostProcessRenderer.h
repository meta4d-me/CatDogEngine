#pragma once

#include "ECWorld/SceneWorld.h"
#include "Renderer.h"
#include<bgfx/bgfx.h>

#define TEX_CHAIN_LEN 5

namespace engine
{

class PostProcessRenderer final : public Renderer
{
public:
	using Renderer::Renderer;
	virtual ~PostProcessRenderer();

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;
	
	virtual void SetEnable(bool value) override;
	virtual bool IsEnable() const override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;

	bgfx::FrameBufferHandle m_texChainFb[TEX_CHAIN_LEN];
	uint8_t capturebrightness_id;
	uint8_t start_downsample_id;
	uint8_t start_upsample_id;

	uint16_t pre_width;
	uint16_t pre_height;
};

}