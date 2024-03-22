#pragma once

#include "Renderer.h"
#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/ParticleForceFieldComponent.h"
#include "ECWorld/TransformComponent.h"
#include "RenderContext.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
namespace engine
{

class SceneWorld;

class ParticleRenderer final : public Renderer
{
public:
	using Renderer::Renderer;

	virtual void Init() override;
	virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }
	float getRandomValue(float min, float max) { return min + static_cast<float>(rand()) / (RAND_MAX / (max - min)); }
	void SetForceFieldRotationForce(ParticleForceFieldComponent* forcefield) { m_forcefieldRotationFoce = forcefield->GetRotationForce(); }
	void SetForceFieldRange(ParticleForceFieldComponent* forcefield ,cd::Vec3f scale) { m_forcefieldRange = forcefield->GetForceFieldRange()*scale; }
private:
	SceneWorld* m_pCurrentSceneWorld = nullptr;
	bgfx::TextureHandle m_particleTextureHandle;
	ParticleType m_currentType = ParticleType::Sprite;

	bool m_forcefieldRotationFoce = false;
	cd::Vec3f m_forcefieldRange;
};

}