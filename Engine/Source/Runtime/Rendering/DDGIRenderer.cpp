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

constexpr const char *lightCountAndStride    = "u_lightCountAndStride";
constexpr const char *lightParams            = "u_lightParams";
constexpr const char *cameraPos              = "u_cameraPos";
constexpr const char *albedoColor            = "u_albedoColor";
constexpr const char *albedoUVOffsetAndScale = "u_albedoUVOffsetAndScale";

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
	case DDGITextureType::Classification:
		textureSize = cd::Vec2f(probeCount.y() * probeCount.z(), probeCount.x()) * CLASSIFICATICON_GRID_SIZE;
		info.m_textureSizeX = static_cast<uint16_t>(textureSize.x());
		info.m_textureSizeY = static_cast<uint16_t>(textureSize.y());
		info.m_format = bgfx::TextureFormat::Enum::R32F;
		info.m_pData = reinterpret_cast<const void *>(pDDGIComponent->GetClassificationRawData());
		info.m_dataSize = pDDGIComponent->GetClassificationSize();
		break;
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
		info.m_pData = reinterpret_cast<const void *>(pDDGIComponent->GetRelocationRawData());
		info.m_dataSize = pDDGIComponent->GetRelocationSize();
		break;
	}

	return info;
}

void CreatDDGITexture(DDGITextureType type, DDGIComponent* pDDGIComponent, RenderContext* pRenderContext)
{
	assert(nullptr != pDDGIComponent && nullptr != pRenderContext);

	DDGITextureInfo info = GetDDGITextureInfo(type, pDDGIComponent);
	if(nullptr != info.m_pData && info.m_dataSize > 0)
	{
		pRenderContext->CreateTexture(info.m_pName, info.m_textureSizeX, info.m_textureSizeY, 1, info.m_format, samplerFlags, info.m_pData, info.m_dataSize);
	}
	else
	{
		CD_ENGINE_WARN("DDGIRenderer faild to create texture {0}!", info.m_pName);
	}
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

	m_pRenderContext->CreateUniform(classificationSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform(distanceSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform(irradianceSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform(relocationSampler, bgfx::UniformType::Sampler);

	// Warning : The coordinate system is different between CD and HWs Engine.
	//   CD: Left-hand, +Y Up
	//   HW: Right-hand, +Z Up
	m_pDDGIComponent->SetVolumeOrigin(cd::Vec3f(0.0f, 0.0f, 0.0f));
	m_pDDGIComponent->SetProbeSpacing(cd::Vec3f(2.0f, 2.0f, 2.0f));
	m_pDDGIComponent->SetProbeCount(cd::Vec3f(4.0f, 2.0f, 5.0f));
	m_pDDGIComponent->SetAmbientMultiplier(1.0);
	m_pDDGIComponent->SetNormalBias(0.0f);
	m_pDDGIComponent->SetViewBias(0.0f);

	m_pDDGIComponent->SetClassificationRawData("ddgi/classification.bin");
	m_pDDGIComponent->SetDistanceRawData("ddgi/distance.bin");
	m_pDDGIComponent->SetIrradianceRawData("ddgi/irradiance.bin");
	m_pDDGIComponent->SetRelocationRawData("ddgi/relocation.bin");

	m_pRenderContext->CreateUniform(volumeOrigin, bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateUniform(volumeProbeSpacing, bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateUniform(volumeProbeCounts, bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateUniform(ambientMultiplier, bgfx::UniformType::Vec4, 1);

	m_pRenderContext->CreateUniform(lightCountAndStride, bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateUniform(lightParams, bgfx::UniformType::Vec4, LightUniform::VEC4_COUNT);
	m_pRenderContext->CreateUniform(cameraPos, bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateUniform(albedoColor, bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateUniform(albedoUVOffsetAndScale, bgfx::UniformType::Vec4, 1);

	CreatDDGITexture(DDGITextureType::Classification, m_pDDGIComponent, m_pRenderContext);
	CreatDDGITexture(DDGITextureType::Distance, m_pDDGIComponent, m_pRenderContext);
	CreatDDGITexture(DDGITextureType::Irradiance, m_pDDGIComponent, m_pRenderContext);
	CreatDDGITexture(DDGITextureType::Relocation, m_pDDGIComponent, m_pRenderContext);
}

void DDGIRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bgfx::setViewFrameBuffer(GetViewID(),*GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void DDGIRenderer::Render(float deltaTime)
{
	const engine::CameraComponent *pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());

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
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle(pMeshComponent->GetVertexBuffer()));
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle(pMeshComponent->GetIndexBuffer()));

		// Material, only albedo texture will be used for ddgi at now.
		for(const auto& [textureType, textureInfo] : pMaterialComponent->GetTextureResources())
		{
			std::optional<MaterialComponent::TextureInfo> optTextureInfo = pMaterialComponent->GetTextureInfo(textureType);
			if(optTextureInfo.has_value())
			{
				const MaterialComponent::TextureInfo& textureInfo = optTextureInfo.value();
				
				if (cd::MaterialTextureType::BaseColor == textureType)
				{
					constexpr StringCrc uvOffsetAndScale("u_albedoUVOffsetAndScale");
					m_pRenderContext->FillUniform(uvOffsetAndScale, &textureInfo.uvOffset, 1);
				}

				bgfx::setTexture(textureInfo.slot, bgfx::UniformHandle(textureInfo.samplerHandle), bgfx::TextureHandle(textureInfo.textureHandle));
			}
		}

		cd::Vec3f tmpAlbedoColor = cd::Vec3f(1.0f, 1.0f, 1.0f);
		m_pRenderContext->FillUniform(StringCrc(albedoColor), tmpAlbedoColor.Begin(), 1);

		m_pRenderContext->FillUniform(StringCrc(cameraPos), &pCameraComponent->GetEye().x(), 1);

		auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();
		size_t lightEntityCount = lightEntities.size();
		static cd::Vec4f lightInfoData(0.0f, LightUniform::LIGHT_STRIDE, 0.0f, 0.0f);
		lightInfoData.x() = static_cast<float>(lightEntityCount);
		m_pRenderContext->FillUniform(StringCrc(lightCountAndStride), lightInfoData.Begin(), 1);

		if (lightEntityCount > 0)
		{
			// Light component storage has continus memory address and layout.
			float *pLightDataBegin = reinterpret_cast<float *>(m_pCurrentSceneWorld->GetLightComponent(lightEntities[0]));
			m_pRenderContext->FillUniform(StringCrc(lightParams), pLightDataBegin, static_cast<uint16_t>(lightEntityCount * LightUniform::LIGHT_STRIDE));
		}

		m_pRenderContext->FillUniform(StringCrc(volumeOrigin), &m_pDDGIComponent->GetVolumeOrigin().x(), 1);
		m_pRenderContext->FillUniform(StringCrc(volumeProbeSpacing), &m_pDDGIComponent->GetProbeSpacing().x(), 1);
		m_pRenderContext->FillUniform(StringCrc(volumeProbeCounts), &m_pDDGIComponent->GetProbeCount().x(), 1);
		cd::Vec4f tmpAmbientMultiplier = cd::Vec4f(m_pDDGIComponent->GetAmbientMultiplier(), 0.0f, 0.0f, 0.0f);
		m_pRenderContext->FillUniform(StringCrc(ambientMultiplier), &tmpAmbientMultiplier, 1);

		UpdateDDGITexture(DDGITextureType::Classification, m_pDDGIComponent, m_pRenderContext);
		UpdateDDGITexture(DDGITextureType::Distance, m_pDDGIComponent, m_pRenderContext);
		UpdateDDGITexture(DDGITextureType::Irradiance, m_pDDGIComponent, m_pRenderContext);
		UpdateDDGITexture(DDGITextureType::Relocation, m_pDDGIComponent, m_pRenderContext);

		bgfx::setTexture(CLA_MAP_SLOT, m_pRenderContext->GetUniform(StringCrc(classificationSampler)),
						 m_pRenderContext->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Classification))));
		bgfx::setTexture(DIS_MAP_SLOT, m_pRenderContext->GetUniform(StringCrc(distanceSampler)),
						 m_pRenderContext->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Distance))));
		bgfx::setTexture(IRR_MAP_SLOT, m_pRenderContext->GetUniform(StringCrc(irradianceSampler)),
						 m_pRenderContext->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Irradiance))));
		bgfx::setTexture(REL_MAP_SLOT, m_pRenderContext->GetUniform(StringCrc(relocationSampler)),
						 m_pRenderContext->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Relocation))));

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		bgfx::submit(GetViewID(), bgfx::ProgramHandle(pMaterialComponent->GetShadingProgram()));
	}
}

}
