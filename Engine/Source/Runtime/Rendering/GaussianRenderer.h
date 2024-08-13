#pragma once

#include "Renderer.h"
#include <vector>

namespace engine
{

class SceneWorld;

class GaussianRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;

	std::vector<float> f_buffer;
	std::vector<int32_t> sizeList;
	std::vector<uint32_t> counts0;
	std::vector<uint32_t> starts0;
	std::vector<uint32_t> depthIndex;

};

}