#pragma once

#include "Renderer.h"

#include <vector>

namespace engine
{

class SceneWorld;

class AABBRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }
	void SetEnableGlobalAABB(bool enable) { m_enableGlobalAABB = enable; }
	bool IsGlobalAABBEnable() const { return m_enableGlobalAABB; }

private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;
	bool m_enableGlobalAABB = false;
};

}