#include "CelluloidRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkyComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "LightUniforms.h"
#include "Material/ShaderSchema.h"
#include "Math/Transform.hpp"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/MeshResource.h"
#include "Rendering/Resources/TextureResource.h"
#include "Scene/Texture.h"
#include "U_IBL.sh"
#include "U_AtmophericScattering.sh"

namespace engine
{
	namespace
	{
		constexpr const char* albedoUVOffsetAndScale = "u_albedoUVOffsetAndScale";
		constexpr const char* dividLine = "u_dividLine";
		constexpr const char* specular = "u_specular";
		constexpr const char* rimLight = "u_rimLight";

		constexpr const char* firstShadowColor = "u_firstShadowColor";
		constexpr const char* secondShadowColor = "u_secondShadowColor";
		constexpr const char* rimLightColor = "u_rimLightColor";

		constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
		constexpr uint64_t defaultRenderingState = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;

	}

	void CelluloidRenderer::Init()
	{
		bgfx::setViewName(GetViewID(), "CelluloidRenderer");
	}

	void CelluloidRenderer::Warmup()
	{

		GetRenderContext()->CreateUniform(dividLine, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(specular, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(firstShadowColor, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(secondShadowColor, bgfx::UniformType:: Vec4, 1);
		GetRenderContext()->CreateUniform(rimLight, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(rimLightColor, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(albedoUVOffsetAndScale, bgfx::UniformType::Vec4, 1);
	}

	void CelluloidRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
	{
		UpdateViewRenderTarget();
		bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
	}

	void CelluloidRenderer::Render(float deltaTime)
	{
		// TODO : Remove it. If every renderer need to submit camera related uniform, it should be done not inside Renderer class.
		const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();
		SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

		for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
		{
			MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
			if (!pMaterialComponent || pMaterialComponent->GetMaterialType() != m_pCurrentSceneWorld->GetCelluloidMaterialType())
			{
				continue;
			}
				bool textureSlotBindTable[32] = { false };
		for (const auto& [textureType, propertyGroup] : pMaterialComponent->GetPropertyGroups())
		{
			const MaterialComponent::TextureInfo& textureInfo = propertyGroup.textureInfo;
			if (textureSlotBindTable[textureInfo.slot])
			{
				// already bind.
				continue;
			}

			TextureResource* pTextureResource = textureInfo.pTextureResource;
			if (!propertyGroup.useTexture ||
				pTextureResource == nullptr ||
				(pTextureResource->GetStatus() != ResourceStatus::Ready && pTextureResource->GetStatus() != ResourceStatus::Optimized))
			{
				continue;
			}

			if (cd::MaterialTextureType::BaseColor == textureType)
			{
				constexpr StringCrc albedoUVOffsetAndScaleCrc(albedoUVOffsetAndScale);
				cd::Vec4f uvOffsetAndScaleData(textureInfo.GetUVOffset().x(), textureInfo.GetUVOffset().y(),
					textureInfo.GetUVScale().x(), textureInfo.GetUVScale().y());
				GetRenderContext()->FillUniform(albedoUVOffsetAndScaleCrc, &uvOffsetAndScaleData, 1);
			}

			textureSlotBindTable[textureInfo.slot] = true;
			bgfx::setTexture(textureInfo.slot, bgfx::UniformHandle{ pTextureResource->GetSamplerHandle() }, bgfx::TextureHandle{ pTextureResource->GetTextureHandle() });
		}
		constexpr StringCrc dividLineCrc(dividLine);
		GetRenderContext()->FillUniform(dividLineCrc, pMaterialComponent->GetToonParameters().dividLine.begin(), 1);

		constexpr StringCrc specularCrc(specular);
		GetRenderContext()->FillUniform(specularCrc, pMaterialComponent->GetToonParameters().specular.begin(), 1);

		constexpr StringCrc firstShadowColorCrc(firstShadowColor);
		GetRenderContext()->FillUniform(firstShadowColorCrc, pMaterialComponent->GetToonParameters().firstShadowColor.begin(), 1);

		constexpr StringCrc secondShadowColorCrc(secondShadowColor);
		GetRenderContext()->FillUniform(secondShadowColorCrc, pMaterialComponent->GetToonParameters().secondShadowColor.begin(), 1);

		constexpr StringCrc rimLightColorCrc(rimLightColor);
		GetRenderContext()->FillUniform(rimLightColorCrc, pMaterialComponent->GetToonParameters().rimLightColor.begin(), 1);

		constexpr StringCrc rimLightCrc(rimLight);
		GetRenderContext()->FillUniform(rimLightCrc, pMaterialComponent->GetToonParameters().rimLight.begin(), 1);

		GetRenderContext()->Submit(GetViewID(), pMaterialComponent->GetShaderProgramName(), pMaterialComponent->GetFeaturesCombine());
		}
	}

}