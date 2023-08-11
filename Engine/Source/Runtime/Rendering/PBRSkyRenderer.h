#pragma once

#include "Renderer.h"

namespace engine
{

class SceneWorld;

class PBRSkyRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;
	virtual bool IsEnable() const override;
	
	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	void Precompute() const;
	void SafeDestroyTexture(const char* str) const;

private:
	bool m_isPrecomputed = false;
	SceneWorld* m_pCurrentSceneWorld = nullptr;
};

}