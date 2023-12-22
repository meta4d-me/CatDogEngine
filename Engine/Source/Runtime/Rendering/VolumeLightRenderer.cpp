#include "VolumeLightRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ECWorld/LightComponent.h"
#include "LightUniforms.h"
#include "Material/ShaderSchema.h"
#include "Math/Transform.hpp"
#include "RenderContext.h"

#include <string>

namespace engine
{

namespace
{
constexpr const char* cameraPos = "u_cameraPos";

constexpr const char* lightCountAndStride = "u_lightCountAndStride";
constexpr const char* lightParams = "u_lightParams";

constexpr const char* lightDir = "u_LightDir";
constexpr const char* heightOffsetAndshadowLength = "u_HeightOffsetAndshadowLength";

constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr uint64_t defaultRenderingState = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;

}

void VolumeLightRenderer::Init()
{
	constexpr StringCrc programCrc = StringCrc("VolumeLightProgram");
	GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_volumeLight", "fs_volumeLight" });

	bgfx::setViewName(GetViewID(), "VolumeLightRenderer");
}

void VolumeLightRenderer::Warmup()
{
	GetRenderContext()->UploadShaderProgram("VolumeLightProgram");
}

void VolumeLightRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();		//TODO
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void VolumeLightRenderer::Render(float deltaTime)
{
	// Submit uniform values : light settings
	/*auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();
	if (!lightEntities.empty())
	{
		// TODO : Remove it. If every renderer need to submit camera related uniform, it should be done not inside Renderer class.
		const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();
		//camera 
		CameraComponent* pMainCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
		const cd::Matrix4x4 camView = pMainCameraComponent->GetViewMatrix();
		const cd::Matrix4x4 camProj = pMainCameraComponent->GetProjectionMatrix();
		const cd::Matrix4x4 invCamViewProj = (camProj * camView).Inverse();

		for (auto lightEntity : lightEntities)
		{
			LightComponent* lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntity);

			if (!lightComponent->IsCastVolume())
			{
				continue;
			}

			switch (lightComponent->GetType())
			{
			case cd::LightType::Directional:
			{
				
			}
			break;
			case cd::LightType::Point:
			{
				
			}
			break;
			case cd::LightType::Spot:
			{
				
			}
			break;
			}
		}
	}*/
}

}
