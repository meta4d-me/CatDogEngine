#include "TerrainRenderer.h"

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
#include "Scene/Texture.h"
#include "U_IBL.sh"
#include "U_Terrain.sh"

namespace engine
{

namespace
{

constexpr const char* snowSampler = "s_texSnow";
constexpr const char* rockSampler = "s_texRock";
constexpr const char* grassSampler = "s_texGrass";
constexpr const char* elevationSampler = "s_texElevation";

constexpr const char* snowTexture = "Textures/terrain/snow_baseColor.dds";
constexpr const char* rockTexture = "Textures/terrain/rock_baseColor.dds";
constexpr const char* grassTexture = "Textures/terrain/grass_baseColor.dds";
constexpr const char* elevationTexture = "Terrain";

constexpr const char* lutSampler = "s_texLUT";
constexpr const char* cubeIrradianceSampler = "s_texCubeIrr";
constexpr const char* cubeRadianceSampler = "s_texCubeRad";
constexpr const char* iblStrength = "u_iblStrength";

constexpr const char* lutTexture = "Textures/lut/ibl_brdf_lut.dds";

constexpr const char* cameraPos = "u_cameraPos";
constexpr const char* cameraNearFarPlane = "u_cameraNearFarPlane";

constexpr const char* albedoColor = "u_albedoColor";
constexpr const char* metallicRoughnessFactor = "u_metallicRoughnessFactor";
constexpr const char* albedoUVOffsetAndScale = "u_albedoUVOffsetAndScale";
constexpr const char* alphaCutOff = "u_alphaCutOff";
constexpr const char* emissiveColor = "u_emissiveColor";

constexpr const char* lightCountAndStride = "u_lightCountAndStride";
constexpr const char* lightParams = "u_lightParams";

constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr uint64_t defaultRenderingState = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;

}

void TerrainRenderer::Init()
{
	bgfx::setViewName(GetViewID(), "TerrainRenderer");
}

void TerrainRenderer::Warmup()
{
	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	GetRenderContext()->CreateUniform(snowSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(rockSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(grassSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(elevationSampler, bgfx::UniformType::Sampler);

	GetRenderContext()->CreateTexture(snowTexture);
	GetRenderContext()->CreateTexture(rockTexture);
	GetRenderContext()->CreateTexture(grassTexture);
	GetRenderContext()->CreateTexture(elevationTexture);

	GetRenderContext()->CreateTexture(lutTexture);
	GetRenderContext()->CreateTexture(pSkyComponent->GetIrradianceTexturePath().c_str(), samplerFlags);
	GetRenderContext()->CreateTexture(pSkyComponent->GetRadianceTexturePath().c_str(), samplerFlags);
	GetRenderContext()->CreateUniform(iblStrength, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->CreateUniform(cameraPos, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(cameraNearFarPlane, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->CreateUniform(albedoColor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(emissiveColor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(metallicRoughnessFactor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(albedoUVOffsetAndScale, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(alphaCutOff, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->CreateUniform(lightCountAndStride, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(lightParams, bgfx::UniformType::Vec4, LightUniform::VEC4_COUNT);

	GetRenderContext()->CreateTexture(elevationTexture, 129U, 129U, 1, bgfx::TextureFormat::Enum::R32F, samplerFlags, nullptr, 0);
}

void TerrainRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void TerrainRenderer::Render(float deltaTime)
{
	// TODO : Remove it. If every renderer need to submit camera related uniform, it should be done not inside Renderer class.
	const CameraComponent* pMainCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
	const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();
	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	for (Entity entity : m_pCurrentSceneWorld->GetTerrainEntities())
	{		
		MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		if (!pMaterialComponent ||
			pMaterialComponent->GetMaterialType() != m_pCurrentSceneWorld->GetTerrainMaterialType() ||
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

		// Transform
		if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}

		// Material
		bgfx::setTexture(TERRAIN_TOP_ALBEDO_MAP_SLOT,
			GetRenderContext()->GetUniform(StringCrc(snowSampler)),
			GetRenderContext()->GetTexture(StringCrc(snowTexture)));

		bgfx::setTexture(TERRAIN_MEDIUM_ALBEDO_MAP_SLOT,
			GetRenderContext()->GetUniform(StringCrc(rockSampler)),
			GetRenderContext()->GetTexture(StringCrc(rockTexture)));

		bgfx::setTexture(TERRAIN_BOTTOM_ALBEDO_MAP_SLOT,
			GetRenderContext()->GetUniform(StringCrc(grassSampler)),
			GetRenderContext()->GetTexture(StringCrc(grassTexture)));

		TerrainComponent* pTerrainComponent = m_pCurrentSceneWorld->GetTerrainComponent(entity);
		GetRenderContext()->UpdateTexture(elevationTexture, 0, 0, 0, 0, 0, pTerrainComponent->GetTexWidth(), pTerrainComponent->GetTexDepth(),
			1, pTerrainComponent->GetElevationRawData(), pTerrainComponent->GetElevationRawDataSize());

		bgfx::setTexture(TERRAIN_ELEVATION_MAP_SLOT,
			GetRenderContext()->GetUniform(StringCrc(elevationSampler)),
			GetRenderContext()->GetTexture(StringCrc(elevationTexture)));

		// Sky
		SkyType crtSkyType = pSkyComponent->GetSkyType();
		if (crtSkyType == SkyType::SkyBox)
		{
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

			constexpr StringCrc lutsamplerCrc(lutSampler);
			constexpr StringCrc luttextureCrc(lutTexture);
			bgfx::setTexture(BRDF_LUT_SLOT, GetRenderContext()->GetUniform(lutsamplerCrc), GetRenderContext()->GetTexture(luttextureCrc));
		}

		// Submit uniform values : camera settings
		constexpr StringCrc cameraPosCrc(cameraPos);
		GetRenderContext()->FillUniform(cameraPosCrc, &cameraTransform.GetTranslation().x(), 1);

		constexpr StringCrc cameraNearFarPlaneCrc(cameraNearFarPlane);
		float cameraNearFarPlanedata[2] { pMainCameraComponent->GetNearPlane(), pMainCameraComponent->GetFarPlane() };
		GetRenderContext()->FillUniform(cameraNearFarPlaneCrc, cameraNearFarPlanedata, 1);

		// Submit  uniform values : material settings
		constexpr StringCrc albedoColorCrc(albedoColor);
		GetRenderContext()->FillUniform(albedoColorCrc, pMaterialComponent->GetFactor<cd::Vec3f>(cd::MaterialPropertyGroup::BaseColor), 1);

		cd::Vec4f metallicRoughnessFactorData(
			*(pMaterialComponent->GetFactor<float>(cd::MaterialPropertyGroup::Metallic)),
			*(pMaterialComponent->GetFactor<float>(cd::MaterialPropertyGroup::Roughness)),
			1.0f, 1.0f);
		constexpr StringCrc mrFactorCrc(metallicRoughnessFactor);
		GetRenderContext()->FillUniform(mrFactorCrc, metallicRoughnessFactorData.begin(), 1);

		constexpr StringCrc emissiveColorCrc(emissiveColor);
		GetRenderContext()->FillUniform(emissiveColorCrc, pMaterialComponent->GetFactor<cd::Vec4f>(cd::MaterialPropertyGroup::Emissive), 1);

		// Submit  uniform values : light settings
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

		bgfx::setState(state);

		SubmitStaticMeshDrawCall(pMeshComponent, GetViewID(), pMaterialComponent->GetShaderProgramName());
	}
}

}