#include "WorldRenderer.h"

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
#include "U_Shadow.sh"

namespace engine
{

namespace
{

// Sampler
constexpr const char* lutSampler						= "s_texLUT";
constexpr const char* cubeIrradianceSampler       = "s_texCubeIrr";
constexpr const char* cubeRadianceSampler         = "s_texCubeRad";

constexpr const char* shadowMapSampler			= "s_texShadowMap";
constexpr const char* cubeShadowMapSampler	= "s_texCubeShadowMap";

// Uniform
constexpr const char* cameraPos						= "u_cameraPos";
constexpr const char* albedoColor					= "u_albedoColor";
constexpr const char* emissiveColor				= "u_emissiveColor";
constexpr const char* metallicRoughnessFactor = "u_metallicRoughnessFactor";

constexpr const char* albedoUVOffsetAndScale = "u_albedoUVOffsetAndScale";
constexpr const char* alphaCutOff = "u_alphaCutOff";

constexpr const char* lightCountAndStride = "u_lightCountAndStride";
constexpr const char* lightParams = "u_lightParams";

constexpr const char* lightDir = "u_LightDir";
constexpr const char* heightOffsetAndshadowLength = "u_HeightOffsetAndshadowLength";

constexpr const char* lightViewProj[4] = {"u_lightViewProj0", "u_lightViewProj1", "u_lightViewProj2", "u_lightViewProj3"};
constexpr const char* cameraNearFarPlane = "u_cameraNearFarPlane";
constexpr const char* cameraLookAt = "u_cameraLookAt";
constexpr const char* clipFrustumDepth  = "u_clipFrustumDepth";

// Texture
constexpr const char* lutTexture = "Textures/lut/ibl_brdf_lut.dds";

constexpr const char* directionShadowMapTexture	= "DirectionShadowMapTexture";
constexpr const char* pointShadowMapTexture			= "PointShadowMapTexture";
constexpr const char* spotShadowMapTexture			= "SpotShadowMapTexture";
	      
// Flags and states
constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr uint64_t defaultRenderingState = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;

constexpr uint64_t blitDstTextureFlags = BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

}

void WorldRenderer::Init()
{
	constexpr StringCrc programCrc = StringCrc("WorldProgram");
	GetRenderContext()->RegisterShaderProgram(programCrc, { "vs_PBR", "fs_PBR" });

	bgfx::setViewName(GetViewID(), "WorldRenderer");
}

void WorldRenderer::Warmup()
{
	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	GetRenderContext()->CreateUniform(lutSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cubeIrradianceSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cubeRadianceSampler, bgfx::UniformType::Sampler);

	GetRenderContext()->CreateUniform(shadowMapSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cubeShadowMapSampler, bgfx::UniformType::Sampler);

	GetRenderContext()->CreateTexture(lutTexture);
	GetRenderContext()->CreateTexture(pSkyComponent->GetIrradianceTexturePath().c_str(), samplerFlags);
	GetRenderContext()->CreateTexture(pSkyComponent->GetRadianceTexturePath().c_str(), samplerFlags);

	GetRenderContext()->CreateUniform(cameraPos, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(albedoColor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(emissiveColor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(metallicRoughnessFactor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(albedoUVOffsetAndScale, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(alphaCutOff, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->CreateUniform(lightCountAndStride, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(lightParams, bgfx::UniformType::Vec4, LightUniform::VEC4_COUNT);

	GetRenderContext()->CreateUniform(lightViewProj[0], bgfx::UniformType::Mat4, 1);
	GetRenderContext()->CreateUniform(lightViewProj[1], bgfx::UniformType::Mat4, 1);
	GetRenderContext()->CreateUniform(lightViewProj[2], bgfx::UniformType::Mat4, 1);
	GetRenderContext()->CreateUniform(lightViewProj[3], bgfx::UniformType::Mat4, 1);
	GetRenderContext()->CreateUniform(cameraNearFarPlane, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(cameraLookAt, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(clipFrustumDepth, bgfx::UniformType::Vec4, 1);
	
	GetRenderContext()->CreateUniform(lightDir, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(heightOffsetAndshadowLength, bgfx::UniformType::Vec4, 1);
}

void WorldRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void WorldRenderer::Render(float deltaTime)
{
	// TODO : Remove it. If every renderer need to submit camera related uniform, it should be done not inside Renderer class.
	const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();
	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();
	size_t lightEntityCount = lightEntities.size();

	// Update light shadow map
	for (int i = 0; i < lightEntityCount; i++)
	{
		auto lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntities[i]);

		cd::LightType lightType = lightComponent->GetType();
		if (cd::LightType::Directional == lightType)
		{
			uint16_t cascadeNum = lightComponent->GetCascadedNum();
			// Create textureCube if not valid
			if (!lightComponent->IsShadowMapTextureValid())
			{
				bgfx::TextureHandle blitDstShadowMapTexture = bgfx::createTextureCube(lightComponent->GetShadowMapSize(),
					false, 1, bgfx::TextureFormat::D32F, blitDstTextureFlags);
				GetRenderContext()->SetTexture(engine::StringCrc(directionShadowMapTexture), blitDstShadowMapTexture);
				lightComponent->SetShadowMapTexture(blitDstShadowMapTexture);
			}
			// Blit RTV(FrameBuffer Texture) to SRV(Texture)
			bgfx::TextureHandle blitDstShadowMapTexture = lightComponent->GetShadowMapTexture();
			for (uint16_t cascadeIdx = 0; cascadeIdx < cascadeNum; ++cascadeIdx)
			{
				bgfx::TextureHandle blitSrcShadowMapTexture = bgfx::getTexture(lightComponent->GetShadowMapFBs().at(cascadeIdx));
				bgfx::blit(GetViewID(), blitDstShadowMapTexture, 0, 0, 0, cascadeIdx, blitSrcShadowMapTexture, 0, 0, 0, 0);
			}
		}
		else if (cd::LightType::Point == lightType)
		{
			// Create textureCube if not valid
			if (!lightComponent->IsShadowMapTextureValid())
			{
				StringCrc blitDstShadowMapTextureName = StringCrc(pointShadowMapTexture);
				bgfx::TextureHandle blitDstShadowMapTexture = bgfx::createTextureCube(lightComponent->GetShadowMapSize(),
					false, 1, bgfx::TextureFormat::R32F, blitDstTextureFlags);
				GetRenderContext()->SetTexture(blitDstShadowMapTextureName, blitDstShadowMapTexture);
				lightComponent->SetShadowMapTexture(blitDstShadowMapTexture);
			}
			// Blit RTV(FrameBuffer Texture) to SRV(Texture)
			bgfx::TextureHandle blitDstShadowMapTexture = lightComponent->GetShadowMapTexture();
			for (uint16_t i = 0; i < 6; ++i)
			{
				bgfx::TextureHandle blitSrcShadowMapTexture = bgfx::getTexture(lightComponent->GetShadowMapFBs().at(i));
				bgfx::blit(GetViewID(), blitDstShadowMapTexture, 0, 0, 0, i, blitSrcShadowMapTexture, 0, 0, 0, 0);
			}
		}
		else if (cd::LightType::Spot == lightType)
		{
			// Create texture2D if not valid
			if (!lightComponent->IsShadowMapTextureValid())
			{
				bgfx::TextureHandle blitDstShadowMapTexture = GetRenderContext()->CreateTexture(spotShadowMapTexture,
					lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize(), 1, bgfx::TextureFormat::D32F, blitDstTextureFlags);
				lightComponent->SetShadowMapTexture(blitDstShadowMapTexture);
			}
			// Blit RTV(FrameBuffer Texture) to SRV(Texture)
			bgfx::TextureHandle blitDstShadowMapTexture = lightComponent->GetShadowMapTexture();
			bgfx::TextureHandle blitSrcShadowMapTexture = bgfx::getTexture(lightComponent->GetShadowMapFBs().at(0));
			bgfx::blit(GetViewID(), blitDstShadowMapTexture, 0, 0, blitSrcShadowMapTexture);
		}
	}

	for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
	{
		MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		if (!pMaterialComponent ||
			pMaterialComponent->GetMaterialType() != m_pCurrentSceneWorld->GetPBRMaterialType())
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
		if(m_pCurrentSceneWorld->GetAnimationComponent(entity))
		{
			continue;
		}

		// Transform
		if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		// Mesh
		UpdateStaticMeshComponent(pMeshComponent);

		// Material
		for (const auto& [textureType, _] : pMaterialComponent->GetTextureResources())
		{
			if (const MaterialComponent::TextureInfo* pTextureInfo = pMaterialComponent->GetTextureInfo(textureType))
			{
				if (cd::MaterialTextureType::BaseColor == textureType)
				{
					constexpr StringCrc albedoUVOffsetAndScaleCrc(albedoUVOffsetAndScale);
					cd::Vec4f uvOffsetAndScaleData(pTextureInfo->GetUVOffset().x(), pTextureInfo->GetUVOffset().y(),
						pTextureInfo->GetUVScale().x(), pTextureInfo->GetUVScale().y());
					GetRenderContext()->FillUniform(albedoUVOffsetAndScaleCrc, &uvOffsetAndScaleData, 1);
				}

				bgfx::setTexture(pTextureInfo->slot, bgfx::UniformHandle{ pTextureInfo->samplerHandle }, bgfx::TextureHandle{ pTextureInfo->textureHandle });
			}
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
			//bgfx::TextureHandle th1 = GetRenderContext()->GetTexture(StringCrc(pSkyComponent->GetRadianceTexturePath()));
			bgfx::setTexture(IBL_RADIANCE_SLOT,
				GetRenderContext()->GetUniform(radSamplerCrc),
				GetRenderContext()->GetTexture(StringCrc(pSkyComponent->GetRadianceTexturePath())));

			constexpr StringCrc lutsamplerCrc(lutSampler);
			constexpr StringCrc luttextureCrc(lutTexture);
			//bgfx::TextureHandle th2 = GetRenderContext()->GetTexture(luttextureCrc);
			bgfx::setTexture(BRDF_LUT_SLOT, GetRenderContext()->GetUniform(lutsamplerCrc), GetRenderContext()->GetTexture(luttextureCrc));
		}
		else if (SkyType::AtmosphericScattering == crtSkyType)
		{
			bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(pSkyComponent->GetATMTransmittanceCrc()), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(ATM_IRRADIANCE_SLOT, GetRenderContext()->GetTexture(pSkyComponent->GetATMIrradianceCrc()), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(ATM_SCATTERING_SLOT, GetRenderContext()->GetTexture(pSkyComponent->GetATMScatteringCrc()), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);

			constexpr StringCrc LightDirCrc(lightDir);
			GetRenderContext()->FillUniform(LightDirCrc, &(pSkyComponent->GetSunDirection().x()), 1);

			constexpr StringCrc HeightOffsetAndshadowLengthCrc(heightOffsetAndshadowLength);
			cd::Vec4f tmpHeightOffsetAndshadowLength = cd::Vec4f(pSkyComponent->GetHeightOffset(), pSkyComponent->GetShadowLength(), 0.0f, 0.0f);
			GetRenderContext()->FillUniform(HeightOffsetAndshadowLengthCrc, &(tmpHeightOffsetAndshadowLength.x()), 1);
		}

		// Submit uniform values : camera settings
		constexpr StringCrc cameraPosCrc(cameraPos);
		GetRenderContext()->FillUniform(cameraPosCrc, &cameraTransform.GetTranslation().x(), 1);

		// Submit uniform values : material settings
		constexpr StringCrc albedoColorCrc(albedoColor);
		GetRenderContext()->FillUniform(albedoColorCrc, pMaterialComponent->GetAlbedoColor().Begin(), 1);

		constexpr StringCrc mrFactorCrc(metallicRoughnessFactor);
		cd::Vec4f metallicRoughnessFactorData(pMaterialComponent->GetMetallicFactor(), pMaterialComponent->GetRoughnessFactor(), 1.0f, 1.0f);
		GetRenderContext()->FillUniform(mrFactorCrc, metallicRoughnessFactorData.Begin(), 1);

		constexpr StringCrc emissiveColorCrc(emissiveColor);
		GetRenderContext()->FillUniform(emissiveColorCrc, pMaterialComponent->GetEmissiveColor().Begin(), 1);

		// Submit uniform values : light settings
		constexpr engine::StringCrc lightCountAndStrideCrc(lightCountAndStride);
		static cd::Vec4f lightInfoData(0, LightUniform::LIGHT_STRIDE, 0.0f, 0.0f);
		lightInfoData.x() = static_cast<float>(lightEntityCount);
		GetRenderContext()->FillUniform(lightCountAndStrideCrc, lightInfoData.Begin(), 1);
		float lightData[20*6] = {0};
		for (uint16_t i = 0U; i < lightEntityCount; ++i)
		{
			LightComponent* lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntities[i]);
			memcpy(&lightData[20*i+0], lightComponent->GetLightUniformData(), sizeof(U_Light));
		}
		constexpr engine::StringCrc lightParamsCrc(lightParams);
		GetRenderContext()->FillUniform(lightParamsCrc, lightData, static_cast<uint16_t>(lightEntityCount * LightUniform::LIGHT_STRIDE));

		// Submit uniform values : shadow map and settings
		for (int i = 0; i < lightEntityCount; i++)
		{
			auto lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntities[i]);
 
			cd::LightType lightType = lightComponent->GetType();
			if (cd::LightType::Directional == lightType)
			{
				uint16_t cascadeNum = lightComponent->GetCascadedNum();
				bgfx::TextureHandle blitDstShadowMapTexture = lightComponent->GetShadowMapTexture();
				
				// Set SRV texture
				constexpr StringCrc shadowMapSamplerCrc(shadowMapSampler);
				bgfx::setTexture(SHADOW_MAP_CUBE_SLOT, GetRenderContext()->GetUniform(shadowMapSamplerCrc), blitDstShadowMapTexture);
				
				constexpr StringCrc lightViewProjCrc[4] = { StringCrc(lightViewProj[0]), StringCrc(lightViewProj[1]),
					StringCrc(lightViewProj[2]), StringCrc(lightViewProj[3]) };
				for (uint16_t matricesIdx = 0U; matricesIdx < cascadeNum; ++matricesIdx)
				{
					GetRenderContext()->FillUniform(lightViewProjCrc[matricesIdx], lightComponent->GetLightViewProjMatrix().at(matricesIdx).Begin(), 1);
				}
				constexpr StringCrc clipFrustumDepthCrc(clipFrustumDepth);
				GetRenderContext()->FillUniform(clipFrustumDepthCrc, lightComponent->GetComputedCascadeSplit(), 1);
				constexpr StringCrc cameraNearFarPlaneCrc(cameraNearFarPlane);
				CameraComponent* pMainCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
				float cameraFarPlane[2] = { pMainCameraComponent->GetNearPlane(), pMainCameraComponent->GetFarPlane() };
				GetRenderContext()->FillUniform(cameraNearFarPlaneCrc, &cameraFarPlane, 1);
				constexpr StringCrc cameraLookAtCrc(cameraLookAt);
				cd::Matrix4x4 CameraViewTransform = pMainCameraComponent->GetViewMatrix();
				cd::Direction CameraViewDirection = cd::Direction(CameraViewTransform.Data(8), CameraViewTransform.Data(9), CameraViewTransform.Data(10));
				GetRenderContext()->FillUniform(cameraLookAtCrc, CameraViewDirection.Begin(), 1);
			}
			else if(cd::LightType::Point== lightType)
			{
				bgfx::TextureHandle blitDstShadowMapTexture = lightComponent->GetShadowMapTexture();
				// Set SRV texture
				constexpr StringCrc cubeShadowMapSamplerCrc(cubeShadowMapSampler);
				constexpr StringCrc shadowMapSamplerCrc(shadowMapSampler);
				bgfx::setTexture(SHADOW_MAP_CUBE_SLOT, GetRenderContext()->GetUniform(cubeShadowMapSamplerCrc), blitDstShadowMapTexture);
				bgfx::setTexture(SHADOW_MAP_SLOT, GetRenderContext()->GetUniform(shadowMapSamplerCrc), bgfx::TextureHandle());
			}
			else if (cd::LightType::Spot == lightType)
			{	
				// Blit RTV(FrameBuffer Texture) to SRV(Texture)
				bgfx::TextureHandle blitDstShadowMapTexture = lightComponent->GetShadowMapTexture();

				constexpr StringCrc shadowMapSamplerCrc(shadowMapSampler);
				bgfx::setTexture(SHADOW_MAP_SLOT, GetRenderContext()->GetUniform(shadowMapSamplerCrc), blitDstShadowMapTexture);
				constexpr StringCrc lightViewProjCrc(lightViewProj[0]);
				GetRenderContext()->FillUniform(lightViewProjCrc, lightComponent->GetLightViewProjMatrix().at(0).Begin(), 1);
			}
		}

		uint64_t state = defaultRenderingState;
		if (!pMaterialComponent->GetTwoSided())
		{
			state |= BGFX_STATE_CULL_CCW;
		}
		bgfx::setState(state);

		if (cd::BlendMode::Mask == pMaterialComponent->GetBlendMode())
		{
			constexpr StringCrc alphaCutOffCrc(alphaCutOff);
			GetRenderContext()->FillUniform(alphaCutOffCrc, &pMaterialComponent->GetAlphaCutOff(), 1);
		}

		GetRenderContext()->Submit(GetViewID(), "WorldProgram", pMaterialComponent->GetFeaturesCombine());
	}
}

}


