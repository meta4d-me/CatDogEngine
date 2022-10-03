#pragma once

#include "MeshRenderData.h"
#include "Renderer.h"

#include <string>

namespace engine
{

class SceneRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView() override;
	virtual void Render(float deltaTime) override;

	void LoadSceneData(std::string sceneFilePath);
};

}