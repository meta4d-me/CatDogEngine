#pragma once

#include "ECWorld/SceneWorld.h"
#include "Renderer.h"
#include<bgfx/bgfx.h>
#include<vector>

namespace engine
{

class SSAORenderer final : public Renderer
{
public:
	using Renderer::Renderer;
	virtual ~SSAORenderer();

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	virtual void SetEnable(bool value) override;
	virtual bool IsEnable() const override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	void GenerateNoiseTexture();

	SceneWorld* m_pCurrentSceneWorld = nullptr;

	uint16_t width, height;

	float ssaoKernel[64 * 4];

	bgfx::FrameBufferHandle m_ssaoFB;
	bgfx::TextureHandle m_noiseTexture;
};
}