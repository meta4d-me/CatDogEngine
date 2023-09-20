#pragma once

#include "Renderer.h"

#include <vector>

namespace engine
{

class SceneWorld;

class WhiteModelRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void PreSubmit() override;
	virtual bool CheckResources() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;
};

}