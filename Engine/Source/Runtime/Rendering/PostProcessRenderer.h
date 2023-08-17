#pragma once

#include "ECWorld/SceneWorld.h"
#include "Renderer.h"
#include<bgfx/bgfx.h>

#define TEX_CHAIN_LEN 9

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
	void Bloom(cd::Matrix4x4 ortho, CameraComponent* pCameraComponent, bgfx::TextureHandle screenTextureHandle);
	void Blur(uint16_t width, uint16_t height, cd::Matrix4x4 ortho, CameraComponent* pCameraComponent, int index);

	uint16_t captureBrightnessPassID;
	uint16_t start_DownSamplePassID;
	uint16_t start_UpSamplePassID;
	uint16_t blurPassID;
	bgfx::FrameBufferHandle  m_bloomFB[TEX_CHAIN_LEN];
	bgfx::FrameBufferHandle  m_blurFB;
	uint16_t width;
	uint16_t height;

	SceneWorld* m_pCurrentSceneWorld = nullptr;
};

}