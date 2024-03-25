#pragma once

#include "Renderer.h"

namespace engine
{
namespace 
{
constexpr uint16_t shadowLightMaxNum = 3U;
constexpr uint16_t shadowTexturePassMaxNum = 6U;
}

class SceneWorld;

class ShadowMapRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;
	uint16_t m_renderPassID[18];
};

}