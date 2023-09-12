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
	virtual void CreateGraphicsResources() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }
	void SetIsRenderSelected(bool isRenderSelected) { m_isRenderSelected = isRenderSelected; }

private:
	void RenderLines(uint32_t entity);

	SceneWorld* m_pCurrentSceneWorld = nullptr;
	bool m_isRenderSelected = true;	//	false : all , true : selected
};

}