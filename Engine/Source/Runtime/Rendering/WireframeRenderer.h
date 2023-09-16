#pragma once

#include "Renderer.h"

#include <vector>

namespace engine
{

class SceneWorld;

class WireframeRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }
	void SetEnableGlobalWireframe(bool enable) { m_enableGlobalWireframe = enable; }

private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;
	bool m_enableGlobalWireframe = false;
};

}