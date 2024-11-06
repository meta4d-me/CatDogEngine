#pragma once

#include "Renderer.h"
#include <vector>

namespace engine
{

class SceneWorld;

class SkeletonRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;
	void Build();
	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;
	std::vector<std::byte> m_vertexBuffer;
	std::vector<std::byte> m_indexBuffer;
	uint16_t m_boneVBH = UINT16_MAX;
	uint16_t m_boneIBH = UINT16_MAX;
	bool hasBuilt = false;
};

}