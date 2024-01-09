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
#include "Scene/Texture.h"
#include "U_IBL.sh"
#include "U_AtmophericScattering.sh"

namespace engine
{
	namespace
	{
		constexpr const char* lutSampler = "s_texLUT";
		constexpr const char* cubeIrradianceSampler = "s_texCubeIrr";
		constexpr const char* cubeRadianceSampler = "s_texCubeRad";

		constexpr const char* lutTexture = "Textures/lut/ibl_brdf_lut.dds";

		constexpr const char* cameraPos = "u_cameraPos";
		constexpr const char* albedoColor = "u_albedoColor";
		constexpr const char* emissiveColorAndFactor = "u_emissiveColorAndFactor";
		constexpr const char* metallicRoughnessFactor = "u_metallicRoughnessFactor";

		constexpr const char* albedoUVOffsetAndScale = "u_albedoUVOffsetAndScale";
		constexpr const char* alphaCutOff = "u_alphaCutOff";

		constexpr const char* lightCountAndStride = "u_lightCountAndStride";
		constexpr const char* lightParams = "u_lightParams";

		constexpr const char* LightDir = "u_LightDir";
		constexpr const char* HeightOffsetAndshadowLength = "u_HeightOffsetAndshadowLength";
		constexpr const char* dividLine = "u_dividLine";
		constexpr const char* specular = "u_specular";
		constexpr const char* rimLight = "u_rimLight";

		constexpr const char* baseColor = "u_baseColor";
		constexpr const char* firstShadowColor = "u_firstShadowColor";
		constexpr const char* secondShadowColor = "u_secondShadowColor";

		constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
		constexpr uint64_t defaultRenderingState = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;

	}

	void CelluloidRenderer::Init()
	{
		/*constexpr StringCrc programCrc = StringCrc("CelluloidProgram");
		GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_celluloid", "fs_celluloid" });*/

		bgfx::setViewName(GetViewID(), "CelluloidRenderer");
	}

	void CelluloidRenderer::Warmup()
	{
		SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

		GetRenderContext()->CreateUniform(lutSampler, bgfx::UniformType::Sampler);
		GetRenderContext()->CreateUniform(cubeIrradianceSampler, bgfx::UniformType::Sampler);
		GetRenderContext()->CreateUniform(cubeRadianceSampler, bgfx::UniformType::Sampler);

		GetRenderContext()->CreateTexture(lutTexture);
		GetRenderContext()->CreateTexture(pSkyComponent->GetIrradianceTexturePath().c_str(), samplerFlags);
		GetRenderContext()->CreateTexture(pSkyComponent->GetRadianceTexturePath().c_str(), samplerFlags);

		GetRenderContext()->CreateUniform(cameraPos, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(albedoColor, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(emissiveColorAndFactor, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(metallicRoughnessFactor, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(albedoUVOffsetAndScale, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(alphaCutOff, bgfx::UniformType::Vec4, 1);

		GetRenderContext()->CreateUniform(lightCountAndStride, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(lightParams, bgfx::UniformType::Vec4, LightUniform::VEC4_COUNT);

		GetRenderContext()->CreateUniform(LightDir, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(HeightOffsetAndshadowLength, bgfx::UniformType::Vec4, 1);

		GetRenderContext()->CreateUniform(dividLine, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(specular, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(baseColor, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(firstShadowColor, bgfx::UniformType::Vec4, 1);
		GetRenderContext()->CreateUniform(secondShadowColor, bgfx::UniformType:: Vec4, 1);
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
				// TODO : improve this condition. As we want to skip some feature-specified entities to render.
				// For example, terrain/particle/...
				continue;
			}

			// No mesh attached?
			StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
			if (!pMeshComponent)
			{
				continue;
			}

			BlendShapeComponent* pBlendShapeComponent = m_pCurrentSceneWorld->GetBlendShapeComponent(entity);
			if (pBlendShapeComponent)
			{
				continue;
			}

			// SkinMesh
			if (m_pCurrentSceneWorld->GetAnimationComponent(entity))
			{
				continue;
			}

			// Transform
			if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
			{
				bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
			}

			// Mesh
			UpdateStaticMeshComponent(pMeshComponent);

			// Material
			for (const auto& [textureType, propertyGroup] : pMaterialComponent->GetPropertyGroups())
			{
				const MaterialComponent::TextureInfo& textureInfo = propertyGroup.textureInfo;

				if (!propertyGroup.useTexture || !bgfx::isValid(bgfx::TextureHandle{ textureInfo.textureHandle }))
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

				bgfx::setTexture(textureInfo.slot, bgfx::UniformHandle{ textureInfo.samplerHandle }, bgfx::TextureHandle{ textureInfo.textureHandle });
			}

			// Sky
			SkyType crtSkyType = pSkyComponent->GetSkyType();
			if (SkyType::SkyBox == crtSkyType)
			{
				// Create a new TextureHandle each frame if the skybox texture path has been updated,
				// otherwise RenderContext::CreateTexture will automatically skip it.

				constexpr StringCrc irrSamplerCrc(cubeIrradianceSampler);
				GetRenderContext()->CreateTexture(pSkyComponent->GetIrradianceTexturePath().c_str(), samplerFlags);
				bgfx::setTexture(IBL_IRRADIANCE_SLOT,
					GetRenderContext()->GetUniform(irrSamplerCrc),
					GetRenderContext()->GetTexture(StringCrc(pSkyComponent->GetIrradianceTexturePath())));

				constexpr StringCrc radSamplerCrc(cubeRadianceSampler);
				GetRenderContext()->CreateTexture(pSkyComponent->GetRadianceTexturePath().c_str(), samplerFlags);
				bgfx::setTexture(IBL_RADIANCE_SLOT,
					GetRenderContext()->GetUniform(radSamplerCrc),
					GetRenderContext()->GetTexture(StringCrc(pSkyComponent->GetRadianceTexturePath())));

				constexpr StringCrc lutsamplerCrc(lutSampler);
				constexpr StringCrc luttextureCrc(lutTexture);
				bgfx::setTexture(BRDF_LUT_SLOT, GetRenderContext()->GetUniform(lutsamplerCrc), GetRenderContext()->GetTexture(luttextureCrc));
			}
			else if (SkyType::AtmosphericScattering == crtSkyType)
			{
				bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(pSkyComponent->GetATMTransmittanceCrc()), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
				bgfx::setImage(ATM_IRRADIANCE_SLOT, GetRenderContext()->GetTexture(pSkyComponent->GetATMIrradianceCrc()), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
				bgfx::setImage(ATM_SCATTERING_SLOT, GetRenderContext()->GetTexture(pSkyComponent->GetATMScatteringCrc()), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);

				constexpr StringCrc LightDirCrc(LightDir);
				GetRenderContext()->FillUniform(LightDirCrc, &(pSkyComponent->GetSunDirection().x()), 1);

				constexpr StringCrc HeightOffsetAndshadowLengthCrc(HeightOffsetAndshadowLength);
				cd::Vec4f tmpHeightOffsetAndshadowLength = cd::Vec4f(pSkyComponent->GetHeightOffset(), pSkyComponent->GetShadowLength(), 0.0f, 0.0f);
				GetRenderContext()->FillUniform(HeightOffsetAndshadowLengthCrc, &(tmpHeightOffsetAndshadowLength.x()), 1);
			}

			// Submit uniform values : camera settings
			constexpr StringCrc cameraPosCrc(cameraPos);
			GetRenderContext()->FillUniform(cameraPosCrc, &cameraTransform.GetTranslation().x(), 1);

			constexpr StringCrc dividLineCrc(dividLine);
			GetRenderContext()->FillUniform(dividLineCrc, pMaterialComponent->GetDividLine().begin(), 1);

			constexpr StringCrc specularCrc(specular);
			GetRenderContext()->FillUniform(specularCrc, pMaterialComponent->GetSpecular().begin(), 1);

			constexpr StringCrc baseColorCrc(baseColor);
			GetRenderContext()->FillUniform(baseColorCrc, pMaterialComponent->GetBaseColor().begin(), 1);

			constexpr StringCrc firstShadowColorCrc(firstShadowColor);
			GetRenderContext()->FillUniform(firstShadowColorCrc, pMaterialComponent->GetFirstShadowColor().begin(), 1);

			constexpr StringCrc secondShadowColorCrc(secondShadowColor);
			GetRenderContext()->FillUniform(secondShadowColorCrc, pMaterialComponent->GetSecondShadowColor().begin(), 1);

			// Submit uniform values : material settings
			constexpr StringCrc albedoColorCrc(albedoColor);
			GetRenderContext()->FillUniform(albedoColorCrc, pMaterialComponent->GetFactor<cd::Vec3f>(cd::MaterialPropertyGroup::BaseColor), 1);

			cd::Vec4f metallicRoughnessFactorData(
				*(pMaterialComponent->GetFactor<float>(cd::MaterialPropertyGroup::Metallic)),
				*(pMaterialComponent->GetFactor<float>(cd::MaterialPropertyGroup::Roughness)),
				1.0f, 1.0f);
			constexpr StringCrc mrFactorCrc(metallicRoughnessFactor);
			GetRenderContext()->FillUniform(mrFactorCrc, metallicRoughnessFactorData.begin(), 1);

			constexpr StringCrc emissiveColorCrc(emissiveColorAndFactor);
			GetRenderContext()->FillUniform(emissiveColorCrc, pMaterialComponent->GetFactor<cd::Vec4f>(cd::MaterialPropertyGroup::Emissive), 1);

			// Submit uniform values : light settings
			auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();
			size_t lightEntityCount = lightEntities.size();
			constexpr engine::StringCrc lightCountAndStrideCrc(lightCountAndStride);
			static cd::Vec4f lightInfoData(0, LightUniform::LIGHT_STRIDE, 0.0f, 0.0f);
			lightInfoData.x() = static_cast<float>(lightEntityCount);
			GetRenderContext()->FillUniform(lightCountAndStrideCrc, lightInfoData.begin(), 1);
			if (lightEntityCount > 0)
			{
				// Light component storage has continus memory address and layout.
				float* pLightDataBegin = reinterpret_cast<float*>(m_pCurrentSceneWorld->GetLightComponent(lightEntities[0]));
				constexpr engine::StringCrc lightParamsCrc(lightParams);
				GetRenderContext()->FillUniform(lightParamsCrc, pLightDataBegin, static_cast<uint16_t>(lightEntityCount * LightUniform::LIGHT_STRIDE));
			}

			uint64_t state = defaultRenderingState;
			if (!pMaterialComponent->GetTwoSided())
			{
				state |= BGFX_STATE_CULL_CCW;
			}

			if (cd::BlendMode::Mask == pMaterialComponent->GetBlendMode())
			{
				constexpr StringCrc alphaCutOffCrc(alphaCutOff);
				GetRenderContext()->FillUniform(alphaCutOffCrc, &pMaterialComponent->GetAlphaCutOff(), 1);
			}

			bgfx::setState(state);

			GetRenderContext()->Submit(GetViewID(), pMaterialComponent->GetShaderProgramName(), pMaterialComponent->GetFeaturesCombine());
		}
	}

}