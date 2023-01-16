#pragma once

#include "ECWorld/Entity.h"
#include "MeshRenderData.h"
#include "Renderer.h"

#include <vector>

namespace engine
{

class World;

class WorldRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetWorld(World* pWorld) { m_pCurrentWorld = pWorld; }

private:
	World* m_pCurrentWorld = nullptr;
	bgfx::ProgramHandle m_programPBR;
	bgfx::ProgramHandle m_programPBR_AO;
};

}