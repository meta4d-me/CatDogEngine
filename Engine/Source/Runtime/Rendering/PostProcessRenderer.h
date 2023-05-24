#pragma once

#include "Renderer.h"
#include <ECWorld/SceneWorld.h>

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
	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
		SceneWorld* m_pCurrentSceneWorld = nullptr;
		float m_fGammaCorrection[4] ;
};

}