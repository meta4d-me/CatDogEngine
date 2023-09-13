#include "ParticleRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "RenderContext.h"

namespace engine {

void ParticleRenderer::Init()
{
	auto* pShaderVariantCollectionsComponent = m_pCurrentSceneWorld->GetShaderVariantCollectionsComponent(m_pCurrentSceneWorld->GetShaderVariantCollectionEntity());
	// pShaderVariantCollectionsComponent->RegisterPragram();

	bgfx::setViewName(GetViewID(), "ParticleRenderer");
}

void ParticleRenderer::CreateGraphicsResources()
{

}

void ParticleRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void ParticleRenderer::Render(float deltaTime)
{
	const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();
}

}