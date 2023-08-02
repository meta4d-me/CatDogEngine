#include "DDGIRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Material/ShaderSchema.h"
#include "RenderContext.h"
#include "Rendering/DDGIDefinition.h"
#include "Scene/Texture.h"
#include "U_DDGI.sh"
#include "U_Environment.sh"

namespace engine
{

namespace
{

constexpr const char* classificationSampler  = "s_texClassification";
constexpr const char* distanceSampler        = "s_texDistance";
constexpr const char* irradianceSampler      = "s_texIrradiance";
constexpr const char* relocationSampler      = "s_texRelocation";
										     
constexpr const char* volumeOrigin           = "u_volumeOrigin";
constexpr const char* volumeProbeSpacing     = "u_volumeProbeSpacing";
constexpr const char* volumeProbeCounts      = "u_volumeProbeCounts";
constexpr const char* ambientMultiplier      = "u_ambientMultiplier";
constexpr const char* normalAndViewBias      = "u_normalAndViewBias";

constexpr const char* lightCountAndStride    = "u_lightCountAndStride";
constexpr const char* lightParams            = "u_lightParams";
constexpr const char* cameraPos              = "u_cameraPos";
constexpr const char* albedoColor            = "u_albedoColor";
constexpr const char* albedoUVOffsetAndScale = "u_albedoUVOffsetAndScale";

constexpr const char* lutSampler = "s_texLUT";
constexpr const char* cubeIrradianceSampler = "s_texCubeIrr";
constexpr const char* cubeRadianceSampler = "s_texCubeRad";

constexpr const char* lutTexture = "Textures/lut/ibl_brdf_lut.dds";

constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;

struct DDGITextureInfo
{
	const char *m_pName = "";
	uint16_t m_textureSizeX = 0;
	uint16_t m_textureSizeY = 0;
	bgfx::TextureFormat::Enum m_format = bgfx::TextureFormat::Enum::Unknown;
	const void *m_pData = nullptr;
	uint32_t m_dataSize = 0;
};

DDGITextureInfo GetDDGITextureInfo(DDGITextureType type, DDGIComponent* pDDGIComponent)
{
	DDGITextureInfo info;

	cd::Vec3f probeCount = pDDGIComponent->GetProbeCount();
	cd::Vec2f textureSize = cd::Vec2f(0.0f, 0.0f);

	info.m_pName = GetDDGITextureTypeName(type);

	switch (type)
	{
	case DDGITextureType::Distance:
		textureSize = cd::Vec2f(probeCount.y() * probeCount.z(), probeCount.x()) * DISTANCE_GRID_SIZE;
		info.m_textureSizeX = static_cast<uint16_t>(textureSize.x());
		info.m_textureSizeY = static_cast<uint16_t>(textureSize.y());
		info.m_format = bgfx::TextureFormat::Enum::RG32F;
		info.m_pData = reinterpret_cast<const void *>(pDDGIComponent->GetDistanceRawData());
		info.m_dataSize = pDDGIComponent->GetDistanceSize();
		break;
	case DDGITextureType::Irradiance:
		textureSize = cd::Vec2f(probeCount.y() * probeCount.z(), probeCount.x()) * IRRADIANCE_GRID_SIZE;
		info.m_textureSizeX = static_cast<uint16_t>(textureSize.x());
		info.m_textureSizeY = static_cast<uint16_t>(textureSize.y());
		info.m_format = bgfx::TextureFormat::Enum::RGBA16F;
		info.m_pData = reinterpret_cast<const void *>(pDDGIComponent->GetIrradianceRawData());
		info.m_dataSize = pDDGIComponent->GetIrradianceSize();
		break;
	case DDGITextureType::Relocation:
		textureSize = cd::Vec2f(probeCount.y() * probeCount.z(), probeCount.x()) * RELOCATION_GRID_SIZE;
		info.m_textureSizeX = static_cast<uint16_t>(textureSize.x());
		info.m_textureSizeY = static_cast<uint16_t>(textureSize.y());
		info.m_format = bgfx::TextureFormat::Enum::RGBA16F;
		info.m_pData = reinterpret_cast<const void*>(pDDGIComponent->GetRelocationRawData());
		info.m_dataSize = pDDGIComponent->GetRelocationSize();
		break;
	case DDGITextureType::Classification:
		textureSize = cd::Vec2f(probeCount.y() * probeCount.z(), probeCount.x()) * CLASSIFICATICON_GRID_SIZE;
		info.m_textureSizeX = static_cast<uint16_t>(textureSize.x());
		info.m_textureSizeY = static_cast<uint16_t>(textureSize.y());
		info.m_format = bgfx::TextureFormat::Enum::R32F;
		info.m_pData = reinterpret_cast<const void *>(pDDGIComponent->GetClassificationRawData());
		info.m_dataSize = pDDGIComponent->GetClassificationSize();
		break;
	default:
		break;
	}

	return info;
}

void CreatDDGITexture(DDGITextureType type, DDGIComponent* pDDGIComponent, RenderContext* pRenderContext)
{
	assert(nullptr != pDDGIComponent && nullptr != pRenderContext);

	DDGITextureInfo info = GetDDGITextureInfo(type, pDDGIComponent);
	// BGFX will create an immutable teture which cant be update when (NULL != _mem).
	pRenderContext->CreateTexture(info.m_pName, info.m_textureSizeX, info.m_textureSizeY, 1, info.m_format, samplerFlags, nullptr, 0);
}

void UpdateDDGITexture(DDGITextureType type, DDGIComponent* pDDGIComponent, RenderContext* pRenderContext)
{
	assert(nullptr != pDDGIComponent && nullptr != pRenderContext);

	DDGITextureInfo info = GetDDGITextureInfo(type, pDDGIComponent);
	if (nullptr != info.m_pData && info.m_dataSize > 0)
	{
		pRenderContext->UpdateTexture(info.m_pName, 0, 0, 0, 0, 0, info.m_textureSizeX, info.m_textureSizeY, 1, info.m_pData, info.m_dataSize);
	}
	else
	{
		CD_ENGINE_WARN("DDGIRenderer faild to update texture {0}!", info.m_pName);
	}
}

}

void DDGIRenderer::Init()	
{
	assert(m_pCurrentSceneWorld && "Unknown Scene World pointer!");
	m_pDDGIComponent = m_pCurrentSceneWorld->GetDDGIComponent(m_pCurrentSceneWorld->GetDDGIEntity());
	assert(m_pDDGIComponent && "Unknown DDGI Component pointer!");

	bgfx::setViewName(GetViewID(), "WorldRenderer");

	GetRenderContext()->CreateUniform(distanceSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(irradianceSampler, bgfx::UniformType::Sampler);
	// GetRenderContext()->CreateUniform(relocationSampler, bgfx::UniformType::Sampler);
	// GetRenderContext()->CreateUniform(classificationSampler, bgfx::UniformType::Sampler);

	// Warning : The coordinate system is different between CD and HWs Engine.
	//   CD: Left-hand, +Y Up
	//   HW: Right-hand, +Z Up

	m_pDDGIComponent->SetVolumeOrigin(cd::Vec3f(1.3f, 5.343f, 0.0f));
	m_pDDGIComponent->SetProbeSpacing(cd::Vec3f(2.0f, 1.0f, 2.0f));
	m_pDDGIComponent->SetProbeCount(cd::Vec3f(15.0f, 12.0f, 10.0f));
	m_pDDGIComponent->SetNormalBias(1.0f);
	m_pDDGIComponent->SetViewBias(0.82f);
	m_pDDGIComponent->SetAmbientMultiplier(0.5f);

	GetRenderContext()->CreateUniform(volumeOrigin, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(volumeProbeSpacing, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(volumeProbeCounts, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(ambientMultiplier, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(normalAndViewBias, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->CreateUniform(lightCountAndStride, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(lightParams, bgfx::UniformType::Vec4, LightUniform::VEC4_COUNT);
	GetRenderContext()->CreateUniform(cameraPos, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(albedoColor, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(albedoUVOffsetAndScale, bgfx::UniformType::Vec4, 1);

	m_pDDGIComponent->ResetTextureRawData(m_pDDGIComponent->GetProbeCount());

	// Now we can get DDGI texture from DDGI SDK every frame without these files.
	m_pDDGIComponent->SetDistanceRawData("ddgi/distance.bin");
	m_pDDGIComponent->SetIrradianceRawData("ddgi/irradiance.bin");
	// m_pDDGIComponent->SetRelocationRawData("ddgi/relocation.bin");
	// m_pDDGIComponent->SetClassificationRawData("ddgi/classification.bin");

	CreatDDGITexture(DDGITextureType::Distance, m_pDDGIComponent, GetRenderContext());
	CreatDDGITexture(DDGITextureType::Irradiance, m_pDDGIComponent, GetRenderContext());
	// CreatDDGITexture(DDGITextureType::Relocation, m_pDDGIComponent, GetRenderContext());
	// CreatDDGITexture(DDGITextureType::Classification, m_pDDGIComponent, GetRenderContext());

	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());
	GetRenderContext()->CreateUniform(lutSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cubeIrradianceSampler, bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform(cubeRadianceSampler, bgfx::UniformType::Sampler);

	GetRenderContext()->CreateTexture(lutTexture);
	GetRenderContext()->CreateTexture(pSkyComponent->GetIrradianceTexturePath().c_str(), samplerFlags);
	GetRenderContext()->CreateTexture(pSkyComponent->GetRadianceTexturePath().c_str(), samplerFlags);
}

void DDGIRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void DDGIRenderer::Render(float deltaTime)
{
	const engine::CameraComponent *pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
	const engine::TransformComponent* pCameraTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity());

	for(Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
	{
		MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		if(!pMaterialComponent || pMaterialComponent->GetMaterialType() != m_pCurrentSceneWorld->GetDDGIMaterialType())
		{
			continue;
		}

		// No mesh attached
		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if(!pMeshComponent)
		{
			continue;
		}

		// Transform
		if(TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		// Mesh
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{pMeshComponent->GetVertexBuffer()});
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{pMeshComponent->GetIndexBuffer()});

		// Material, only albedo texture will be used for ddgi at now.
		for(const auto& [textureType, _] : pMaterialComponent->GetTextureResources())
		{
			if (const MaterialComponent::TextureInfo* pTextureInfo = pMaterialComponent->GetTextureInfo(textureType))
			{
				if (cd::MaterialTextureType::BaseColor == textureType)
				{
					constexpr StringCrc uvOffsetAndScale("u_albedoUVOffsetAndScale");
					GetRenderContext()->FillUniform(uvOffsetAndScale, &pTextureInfo->uvOffset, 1);
				}

				bgfx::setTexture(pTextureInfo->slot, bgfx::UniformHandle{pTextureInfo->samplerHandle}, bgfx::TextureHandle{pTextureInfo->textureHandle});
			}
		}

		cd::Vec3f tmpAlbedoColor = cd::Vec3f(1.0f, 1.0f, 1.0f);
		GetRenderContext()->FillUniform(StringCrc(albedoColor), tmpAlbedoColor.Begin(), 1);

		GetRenderContext()->FillUniform(StringCrc(cameraPos), &pCameraTransformComponent->GetTransform().GetTranslation().x(), 1);

		auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();
		size_t lightEntityCount = lightEntities.size();
		static cd::Vec4f lightInfoData(0.0f, LightUniform::LIGHT_STRIDE, 0.0f, 0.0f);
		lightInfoData.x() = static_cast<float>(lightEntityCount);
		GetRenderContext()->FillUniform(StringCrc(lightCountAndStride), lightInfoData.Begin(), 1);

		if (lightEntityCount > 0)
		{
			// Light component storage has continus memory address and layout.
			float *pLightDataBegin = reinterpret_cast<float *>(m_pCurrentSceneWorld->GetLightComponent(lightEntities[0]));
			GetRenderContext()->FillUniform(StringCrc(lightParams), pLightDataBegin, static_cast<uint16_t>(lightEntityCount * LightUniform::LIGHT_STRIDE));
		}

		GetRenderContext()->FillUniform(StringCrc(volumeOrigin), &m_pDDGIComponent->GetVolumeOrigin().x(), 1);
		GetRenderContext()->FillUniform(StringCrc(volumeProbeSpacing), &m_pDDGIComponent->GetProbeSpacing().x(), 1);
		GetRenderContext()->FillUniform(StringCrc(volumeProbeCounts), &m_pDDGIComponent->GetProbeCount().x(), 1);
		cd::Vec4f tmpAmbientMultiplier = cd::Vec4f(m_pDDGIComponent->GetAmbientMultiplier(), 0.0f, 0.0f, 0.0f);
		GetRenderContext()->FillUniform(StringCrc(ambientMultiplier), &tmpAmbientMultiplier, 1);
		cd::Vec4f tmpNormalAndViewBias = cd::Vec4f(m_pDDGIComponent->GetNormalBias(), m_pDDGIComponent->GetViewBias(), 0.0f, 0.0f);
		GetRenderContext()->FillUniform(StringCrc(normalAndViewBias), &tmpNormalAndViewBias, 1);

		UpdateDDGITexture(DDGITextureType::Distance, m_pDDGIComponent, GetRenderContext());
		UpdateDDGITexture(DDGITextureType::Irradiance, m_pDDGIComponent, GetRenderContext());
		// UpdateDDGITexture(DDGITextureType::Relocation, m_pDDGIComponent, GetRenderContext());
		// UpdateDDGITexture(DDGITextureType::Classification, m_pDDGIComponent, GetRenderContext());

		bgfx::setTexture(DIS_MAP_SLOT, GetRenderContext()->GetUniform(StringCrc(distanceSampler)),
			GetRenderContext()->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Distance))));
		bgfx::setTexture(IRR_MAP_SLOT, GetRenderContext()->GetUniform(StringCrc(irradianceSampler)),
			GetRenderContext()->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Irradiance))));
		// bgfx::setTexture(REL_MAP_SLOT, GetRenderContext()->GetUniform(StringCrc(relocationSampler)),
		// 	GetRenderContext()->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Relocation))));
		// bgfx::setTexture(CLA_MAP_SLOT, GetRenderContext()->GetUniform(StringCrc(classificationSampler)),
		// 	GetRenderContext()->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Classification))));

		SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());
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

		constexpr uint64_t defaultState = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		uint64_t state = defaultState;
		if (!pMaterialComponent->GetTwoSided())
		{
			state |= BGFX_STATE_CULL_CCW;
		}
		bgfx::setState(state);

		bgfx::submit(GetViewID(), bgfx::ProgramHandle{pMaterialComponent->GetShadreProgram()});
	}
}

}