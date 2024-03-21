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
#include "Rendering/Resources/MeshResource.h"
#include "Rendering/Resources/TextureResource.h"
#include "Scene/Texture.h"
#include "U_AtmophericScattering.sh"
#include "U_IBL.sh"
#include "U_Shadow.sh"

namespace engine
{

namespace
{

constexpr const char* lutSampler                  = "s_texLUT";
constexpr const char* cubeIrradianceSampler       = "s_texCubeIrr";
constexpr const char* cubeRadianceSampler         = "s_texCubeRad";
											      
constexpr const char* lutTexture                  = "Textures/lut/ibl_brdf_lut.dds";
											      
constexpr const char* cameraPos                   = "u_cameraPos";
constexpr const char* iblStrength                 = "u_iblStrength";
constexpr const char* albedoColor                 = "u_albedoColor";
constexpr const char* emissiveColorAndFactor      = "u_emissiveColorAndFactor";
constexpr const char* metallicRoughnessFactor     = "u_metallicRoughnessFactor";
											      
constexpr const char* albedoUVOffsetAndScale      = "u_albedoUVOffsetAndScale";
constexpr const char* alphaCutOff                 = "u_alphaCutOff";
											      
constexpr const char* lightCountAndStride         = "u_lightCountAndStride";
constexpr const char* lightParams                 = "u_lightParams";
											      
constexpr const char* LightDir                    = "u_LightDir";
constexpr const char* HeightOffsetAndshadowLength = "u_HeightOffsetAndshadowLength";

constexpr const char* lightViewProjs              = "u_lightViewProjs";
constexpr const char* cubeShadowMapSamplers[3]    = { "s_texCubeShadowMap_1", "s_texCubeShadowMap_2" ,  "s_texCubeShadowMap_3" };

constexpr const char* cameraNearFarPlane          = "u_cameraNearFarPlane";
constexpr const char* cameraLookAt                = "u_cameraLookAt";
constexpr const char* clipFrustumDepth            = "u_clipFrustumDepth";

constexpr const char* directionShadowMapTexture   = "DirectionShadowMapTexture";
constexpr const char* pointShadowMapTexture       = "PointShadowMapTexture";
constexpr const char* spotShadowMapTexture        = "SpotShadowMapTexture";

constexpr uint64_t samplerFlags          = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr uint64_t defaultRenderingState = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
constexpr uint64_t blitDstTextureFlags   = BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

}

void WorldRenderer::Init()
{
	bgfx::setViewName(GetViewID(), "WorldRenderer");
}

void WorldRenderer::Warmup()
{
	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	GetRenderContext()->CreateUniform(lutSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cubeIrradianceSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cubeRadianceSampler, bgfx::UniformType::Sampler);

	GetRenderContext()->CreateTexture(lutTexture, samplerFlags);
	GetRenderContext()->CreateTexture(pSkyComponent->GetIrradianceTexturePath().c_str(), samplerFlags);
	GetRenderContext()->CreateTexture(pSkyComponent->GetRadianceTexturePath().c_str(), samplerFlags);

	GetRenderContext()->CreateUniform(cameraPos, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(iblStrength, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(albedoColor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(emissiveColorAndFactor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(metallicRoughnessFactor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(albedoUVOffsetAndScale, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(alphaCutOff, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->CreateUniform(lightCountAndStride, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(lightParams, bgfx::UniformType::Vec4, LightUniform::VEC4_COUNT);

	GetRenderContext()->CreateUniform(LightDir, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(HeightOffsetAndshadowLength, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->CreateUniform(lightViewProjs, bgfx::UniformType::Mat4, 12);
	GetRenderContext()->CreateUniform(cubeShadowMapSamplers[0], bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cubeShadowMapSamplers[1], bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cubeShadowMapSamplers[2], bgfx::UniformType::Sampler);

	GetRenderContext()->CreateUniform(cameraNearFarPlane, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(clipFrustumDepth, bgfx::UniformType::Vec4, 1);
}

void WorldRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void WorldRenderer::Render(float deltaTime)
{
	// TODO : Remove it. If every renderer need to submit camera related uniform, it should be done not inside Renderer class.
	const CameraComponent* pMainCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
	const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();
	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();
	size_t lightEntityCount = lightEntities.size();

	// Blit RTV to SRV to update light shadow map
	for (int i = 0; i < lightEntityCount; i++)
	{
		auto lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntities[i]);
		cd::LightType lightType = lightComponent->GetType();
		if (cd::LightType::Directional == lightType)
		{
			uint16_t cascadeNum = lightComponent->GetCascadeNum();
			// Create textureCube if not valid
			if (!lightComponent->IsShadowMapTextureValid())
			{
				bgfx::TextureHandle blitDstShadowMapTexture = bgfx::createTextureCube(lightComponent->GetShadowMapSize(),
					false, 1, bgfx::TextureFormat::D32F, blitDstTextureFlags);
				GetRenderContext()->SetTexture(engine::StringCrc(directionShadowMapTexture), blitDstShadowMapTexture);
				lightComponent->SetShadowMapTexture(blitDstShadowMapTexture.idx);
			}
			// Blit RTV(FrameBuffer Texture) to SRV(Texture)
			bgfx::TextureHandle blitDstShadowMapTexture = static_cast<bgfx::TextureHandle>(lightComponent->GetShadowMapTexture());
			for (uint16_t cascadeIdx = 0; cascadeIdx < cascadeNum; ++cascadeIdx)
			{
				bgfx::TextureHandle blitSrcShadowMapTexture = bgfx::getTexture(static_cast<bgfx::FrameBufferHandle>(lightComponent->GetShadowMapFBs().at(cascadeIdx)));
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
				lightComponent->SetShadowMapTexture(blitDstShadowMapTexture.idx);
			}
			// Blit RTV(FrameBuffer Texture) to SRV(Texture)
			bgfx::TextureHandle blitDstShadowMapTexture = static_cast<bgfx::TextureHandle>(lightComponent->GetShadowMapTexture());
			for (uint16_t i = 0; i < 6; ++i)
			{
				bgfx::TextureHandle blitSrcShadowMapTexture = bgfx::getTexture(static_cast<bgfx::FrameBufferHandle>(lightComponent->GetShadowMapFBs().at(i)));
				bgfx::blit(GetViewID(), blitDstShadowMapTexture, 0, 0, 0, i, blitSrcShadowMapTexture, 0, 0, 0, 0);
			}
		}
		else if (cd::LightType::Spot == lightType)
		{
			// Create texture2D if not valid
			if (!lightComponent->IsShadowMapTextureValid())
			{
				bgfx::TextureHandle blitDstShadowMapTexture = bgfx::createTextureCube(lightComponent->GetShadowMapSize(),
					false, 1, bgfx::TextureFormat::D32F, blitDstTextureFlags);
				lightComponent->SetShadowMapTexture(blitDstShadowMapTexture.idx);
			}
			// Blit RTV(FrameBuffer Texture) to SRV(Texture)
			bgfx::TextureHandle blitDstShadowMapTexture = static_cast<bgfx::TextureHandle>(lightComponent->GetShadowMapTexture());
			bgfx::TextureHandle blitSrcShadowMapTexture = bgfx::getTexture(static_cast<bgfx::FrameBufferHandle>(lightComponent->GetShadowMapFBs().at(0)));
			bgfx::blit(GetViewID(), blitDstShadowMapTexture, 0, 0, 0, 0, blitSrcShadowMapTexture, 0, 0, 0, 0);
		}
	}

	for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
	{
		MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		if (!pMaterialComponent ||
			(pMaterialComponent->GetMaterialType() == m_pCurrentSceneWorld->GetPBRMaterialType() && pMaterialComponent->GetMaterialType() == m_pCurrentSceneWorld->GetCelluloidMaterialType()) ||
			!GetRenderContext()->IsShaderProgramValid(pMaterialComponent->GetShaderProgramName(), pMaterialComponent->GetFeaturesCombine()))
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

		const MeshResource* pMeshResource = pMeshComponent->GetMeshResource();
		if (ResourceStatus::Ready != pMeshResource->GetStatus() &&
			ResourceStatus::Optimized != pMeshResource->GetStatus())
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
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}

		// Material
		// TODO : need to check if one texture binds twice to different slot. Or will get bgfx assert about duplicated uniform set.
		// So please have a research about same texture handle binds to different slots multiple times.
		// The factor is to build slot -> texture handle maps before update.
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

		// Sky
		SkyType crtSkyType = pSkyComponent->GetSkyType();
		if (SkyType::SkyBox == crtSkyType)
		{
			// Create a new TextureHandle each frame if the skybox texture path has been updated,
			// otherwise RenderContext::CreateTexture will skip it automatically.

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

			constexpr StringCrc iblStrengthCrc{ iblStrength };
			GetRenderContext()->FillUniform(iblStrengthCrc, &(pMaterialComponent->GetIblStrengeth()));

			constexpr StringCrc lutsamplerCrc{ lutSampler };
			constexpr StringCrc luttextureCrc{ lutTexture };
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

		constexpr StringCrc cameraNearFarPlaneCrc(cameraNearFarPlane);
		float cameraNearFarPlanedata[2]{ pMainCameraComponent->GetNearPlane(), pMainCameraComponent->GetFarPlane() };
		GetRenderContext()->FillUniform(cameraNearFarPlaneCrc, cameraNearFarPlanedata, 1);

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

		// Submit light data
		constexpr engine::StringCrc lightCountAndStrideCrc(lightCountAndStride);
		static cd::Vec4f lightInfoData(0, LightUniform::LIGHT_STRIDE, 0.0f, 0.0f);
		lightInfoData.x() = static_cast<float>(lightEntityCount);
		GetRenderContext()->FillUniform(lightCountAndStrideCrc, lightInfoData.begin(), 1);
		int totalLightViewProjOffset = 0;
		float lightData[4 *7 * 3] = { 0 };
		for (uint16_t i = 0U; i < lightEntityCount; ++i)
		{
			LightComponent* lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntities[i]);
			if (cd::LightType::Directional == lightComponent->GetType())
			{
				lightComponent->SetLightViewProjOffset(totalLightViewProjOffset);
				totalLightViewProjOffset += 4;
			}
			else if (cd::LightType::Spot == lightComponent->GetType())
			{
				lightComponent->SetLightViewProjOffset(totalLightViewProjOffset);
				totalLightViewProjOffset++;
			}
			memcpy(&lightData[4 * 7 * i], lightComponent->GetLightUniformData(), sizeof(U_Light));
		}
		constexpr engine::StringCrc lightParamsCrc(lightParams);
		GetRenderContext()->FillUniform(lightParamsCrc, lightData, static_cast<uint16_t>(lightEntityCount * LightUniform::LIGHT_STRIDE));

		// Submit light view&projection transform
		std::vector<cd::Matrix4x4> lightViewProjsData;
		for (uint16_t i = 0U; i < lightEntityCount; ++i)
		{
			LightComponent* lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntities[i]);
			const std::vector<cd::Matrix4x4>& lightViewProjs = lightComponent->GetLightViewProjMatrix();
			for (auto lightViewProj : lightViewProjs)
			{
				lightViewProjsData.push_back(lightViewProj);
			}
		}
		constexpr engine::StringCrc lightViewProjsCrc(lightViewProjs);
		GetRenderContext()->FillUniform(lightViewProjsCrc, lightViewProjsData.data(), totalLightViewProjOffset);

		// Submit shadow map and settings of each light
		constexpr StringCrc shadowMapSamplerCrcs[3] = { StringCrc(cubeShadowMapSamplers[0]), StringCrc(cubeShadowMapSamplers[1]), StringCrc(cubeShadowMapSamplers[2]) };
		for (int lightIndex = 0; lightIndex < lightEntityCount; lightIndex++)
		{
			auto lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntities[lightIndex]);
			cd::LightType lightType = lightComponent->GetType();
			if (cd::LightType::Directional == lightType)
			{
				bgfx::TextureHandle blitDstShadowMapTexture = static_cast<bgfx::TextureHandle>(lightComponent->GetShadowMapTexture());
				bgfx::setTexture(SHADOW_MAP_CUBE_FIRST_SLOT+lightIndex, GetRenderContext()->GetUniform(shadowMapSamplerCrcs[lightIndex]), blitDstShadowMapTexture);
				// TODO : manual 
				constexpr StringCrc clipFrustumDepthCrc(clipFrustumDepth);
				GetRenderContext()->FillUniform(clipFrustumDepthCrc, lightComponent->GetComputedCascadeSplit(), 1);
			}
			else if (cd::LightType::Point == lightType)
			{
				bgfx::TextureHandle blitDstShadowMapTexture = static_cast<bgfx::TextureHandle>(lightComponent->GetShadowMapTexture());
				bgfx::setTexture(SHADOW_MAP_CUBE_FIRST_SLOT+lightIndex, GetRenderContext()->GetUniform(shadowMapSamplerCrcs[lightIndex]), blitDstShadowMapTexture);
			}
			else if (cd::LightType::Spot == lightType)
			{
				// Blit RTV(FrameBuffer Texture) to SRV(Texture)
				bgfx::TextureHandle blitDstShadowMapTexture = static_cast<bgfx::TextureHandle>(lightComponent->GetShadowMapTexture());
				bgfx::setTexture(SHADOW_MAP_CUBE_FIRST_SLOT+lightIndex, GetRenderContext()->GetUniform(shadowMapSamplerCrcs[lightIndex]), blitDstShadowMapTexture);
			}
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

		// Mesh
		if (BlendShapeComponent* pBlendShapeComponent = m_pCurrentSceneWorld->GetBlendShapeComponent(entity))
		{
			bgfx::setVertexBuffer(0, bgfx::DynamicVertexBufferHandle{ pBlendShapeComponent->GetFinalMorphAffectedVB() });
			bgfx::setVertexBuffer(1, bgfx::VertexBufferHandle{ pBlendShapeComponent->GetNonMorphAffectedVB() });
			// TODO : BlendShape + multiple index buffers.
			bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pMeshComponent->GetMeshResource()->GetIndexBufferHandle(0U) });
			GetRenderContext()->Submit(GetViewID(), pMaterialComponent->GetShaderProgramName(), pMaterialComponent->GetFeaturesCombine());
		}
		else
		{
			SubmitStaticMeshDrawCall(pMeshComponent, GetViewID(), pMaterialComponent->GetShaderProgramName(), pMaterialComponent->GetFeaturesCombine());
		}
	}
}

}