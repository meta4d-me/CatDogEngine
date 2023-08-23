#pragma once

#include "Renderer.h"

#include "ECWorld/DDGIComponent.h"

namespace engine
{

class SceneWorld;

class DDGIRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;
	DDGIComponent* m_pDDGIComponent = nullptr;
};

}