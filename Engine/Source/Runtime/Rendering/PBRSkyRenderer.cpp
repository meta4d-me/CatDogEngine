#include "PBRSkyRenderer.h"

#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkyComponent.h"
#include "Log/Log.h"
#include "Math/Box.hpp"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/MeshResource.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Scene/Mesh.h"
#include "Scene/VertexFormat.h"
#include "U_AtmophericScattering.sh"

namespace engine
{

namespace
{

constexpr const char* ProgramAtmosphericScatteringLUT    = "ProgramAtmosphericScatteringLUT";
constexpr const char* ProgramSingleScatteringRayMarching = "ProgramSingleScatteringRayMarching";

// Compute shaders
constexpr const char* ProgramComputeTransmittance        = "ProgramComputeTransmittance";
constexpr const char* ProgramComputeDirectIrradiance     = "ProgramComputeDirectIrradiance";
constexpr const char* ProgramComputeSingleScattering     = "ProgramComputeSingleScattering";
constexpr const char* ProgramComputeScatteringDensity    = "ProgramComputeScatteringDensity";
constexpr const char* ProgramComputeIndirectIrradiance   = "ProgramComputeIndirectIrradiance";
constexpr const char* ProgramComputeMultipleScattering   = "ProgramComputeMultipleScattering";

constexpr StringCrc ProgramAtmosphericScatteringLUTCrc{ ProgramAtmosphericScatteringLUT };
constexpr StringCrc ProgramSingleScatteringRayMarchingCrc{ ProgramSingleScatteringRayMarching };
constexpr StringCrc ProgramComputeTransmittanceCrc{ ProgramComputeTransmittance };
constexpr StringCrc ProgramComputeDirectIrradianceCrc{ ProgramComputeDirectIrradiance };
constexpr StringCrc ProgramComputeSingleScatteringCrc{ ProgramComputeSingleScattering };
constexpr StringCrc ProgramComputeScatteringDensityCrc{ ProgramComputeScatteringDensity };
constexpr StringCrc ProgramComputeIndirectIrradianceCrc{ ProgramComputeIndirectIrradiance };
constexpr StringCrc ProgramComputeMultipleScatteringCrc{ ProgramComputeMultipleScattering };

constexpr const char* TextureTransmittance               = "TextureTransmittance";
constexpr const char* TextureIrradiance                  = "TextureIrradiance";
constexpr const char* TextureScattering                  = "TextureScattering";
constexpr const char* TextureDeltaIrradiance             = "TextureDeltaIrradiance";
constexpr const char* TextureDeltaRayleighScattering     = "TextureDeltaRayleighScattering";
constexpr const char* TextureDeltaMieScattering          = "TextureDeltaMieScattering";
constexpr const char* TextureDeltaScatteringDensity      = "TextureDeltaScatteringDensity";
constexpr const char* TextureDeltaMultipleScattering     = "TextureDeltaMultipleScattering";

constexpr const char* LightDir                           = "u_LightDir";
constexpr const char* CameraPos                          = "u_cameraPos";
constexpr const char* HeightOffset                       = "u_HeightOffset";
constexpr const char* NumScatteringOrders                = "u_numScatteringOrders";
										                 
constexpr uint64_t FlagTexture2D                         = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
constexpr uint64_t FlagTexture3D                         = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_UVW_CLAMP;
constexpr uint64_t StateRendering                        = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LEQUAL;
constexpr uint16_t ScatteringOrders                      = 6;

}

void PBRSkyRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ProgramAtmosphericScatteringLUT", "vs_atmSkyBox", "fs_PrecomputedAtmosphericScattering_LUT"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ProgramSingleScatteringRayMarching", "vs_atmSkyBox", "fs_SingleScattering_RayMarching"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ProgramComputeTransmittance", "cs_ComputeTransmittance", ShaderProgramType::Compute));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ProgramComputeDirectIrradiance", "cs_ComputeDirectIrradiance", ShaderProgramType::Compute));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ProgramComputeSingleScattering", "cs_ComputeSingleScattering", ShaderProgramType::Compute));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ProgramComputeScatteringDensity", "cs_ComputeScatteringDensity", ShaderProgramType::Compute));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ProgramComputeIndirectIrradiance", "cs_ComputeIndirectIrradiance", ShaderProgramType::Compute));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ProgramComputeMultipleScattering", "cs_ComputeMultipleScattering", ShaderProgramType::Compute));

	GetRenderContext()->CreateTexture(TextureTransmittance, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, 1,
		bgfx::TextureFormat::RGBA32F, FlagTexture2D);
	GetRenderContext()->CreateTexture(TextureIrradiance, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 1,
		bgfx::TextureFormat::RGBA32F, FlagTexture2D);
	GetRenderContext()->CreateTexture(TextureDeltaIrradiance, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 1,
		bgfx::TextureFormat::RGBA32F, FlagTexture2D);
	GetRenderContext()->CreateTexture(TextureDeltaRayleighScattering, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH,
		bgfx::TextureFormat::RGBA32F, FlagTexture3D);
	GetRenderContext()->CreateTexture(TextureDeltaMieScattering, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH,
		bgfx::TextureFormat::RGBA32F, FlagTexture3D);
	GetRenderContext()->CreateTexture(TextureScattering, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH,
		bgfx::TextureFormat::RGBA32F, FlagTexture3D);
	GetRenderContext()->CreateTexture(TextureDeltaScatteringDensity, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH,
		bgfx::TextureFormat::RGBA32F, FlagTexture3D);
	GetRenderContext()->CreateTexture(TextureDeltaMultipleScattering, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH,
		bgfx::TextureFormat::RGBA32F, FlagTexture3D);

	GetRenderContext()->CreateUniform(LightDir, bgfx::UniformType::Enum::Vec4, 1);
	GetRenderContext()->CreateUniform(CameraPos, bgfx::UniformType::Enum::Vec4, 1);
	GetRenderContext()->CreateUniform(HeightOffset, bgfx::UniformType::Enum::Vec4, 1);
	GetRenderContext()->CreateUniform(NumScatteringOrders, bgfx::UniformType::Enum::Vec4, 1);

	bgfx::setViewName(GetViewID(), "PBRSkyRenderer");
}

void PBRSkyRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	if (!IsEnable())
	{
		return;
	}

	// We want the skybox to be centered around the player
	// so that no matter how far the player moves, the skybox won't get any closer.
	// Remove the translation part of the view matrix
	// so only rotation will affect the skybox's position vectors.
	float fixedViewMatrix[16];
	std::memcpy(fixedViewMatrix, pViewMatrix, 12 * sizeof(float));
	fixedViewMatrix[12] = fixedViewMatrix[13] = fixedViewMatrix[14] = 0.0f;
	fixedViewMatrix[15] = 1.0f;

	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), fixedViewMatrix, pProjectionMatrix);
	bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
}

void PBRSkyRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	if (!IsEnable())
	{
		return;
	}

	if (!m_isPrecomputed)
	{
		Precompute();
		m_isPrecomputed = true;
	}

	StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(m_pCurrentSceneWorld->GetSkyEntity());
	if (!pMeshComponent)
	{
		return;
	}

	const MeshResource* pMeshResource = pMeshComponent->GetMeshResource();
	if (ResourceStatus::Ready != pMeshResource->GetStatus() &&
		ResourceStatus::Optimized != pMeshResource->GetStatus())
	{
		return;
	}

	bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureTransmittance)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(ATM_IRRADIANCE_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureIrradiance)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(ATM_SCATTERING_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureScattering)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);

	constexpr StringCrc cameraPosCrc(CameraPos);
	GetRenderContext()->FillUniform(cameraPosCrc, &(m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform().GetTranslation().x()), 1);

	auto skyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	constexpr StringCrc LightDirCrc(LightDir);
	GetRenderContext()->FillUniform(LightDirCrc, &(skyComponent->GetSunDirection().x()), 1);

	constexpr StringCrc HeightOffsetCrc(HeightOffset);
	cd::Vec4f tmpHeightOffset = cd::Vec4f(skyComponent->GetHeightOffset(), 0.0f, 0.0f, 0.0f);
	GetRenderContext()->FillUniform(HeightOffsetCrc, &(tmpHeightOffset.x()), 1);

	bgfx::setState(StateRendering);

	SubmitStaticMeshDrawCall(pMeshComponent, GetViewID(), ProgramAtmosphericScatteringLUTCrc);
}

bool PBRSkyRenderer::IsEnable() const
{
	return SkyType::AtmosphericScattering == m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity())->GetSkyType();
}

void PBRSkyRenderer::Precompute() const
{
	constexpr StringCrc NumScatteringOrdersCrc(NumScatteringOrders);

	constexpr StringCrc ProgramComputeTransmittanceCrc(ProgramComputeTransmittance);
	constexpr StringCrc ProgramComputeDirectIrradianceCrc(ProgramComputeDirectIrradiance);
	constexpr StringCrc ProgramComputeSingleScatteringCrc(ProgramComputeSingleScattering);
	constexpr StringCrc ProgramComputeScatteringDensityCrc(ProgramComputeScatteringDensity);
	constexpr StringCrc ProgramComputeIndirectIrradianceCrc(ProgramComputeIndirectIrradiance);
	constexpr StringCrc ProgramComputeMultipleScatteringCrc(ProgramComputeMultipleScattering);

	constexpr StringCrc TextureTransmittanceCrc(TextureTransmittance);
	constexpr StringCrc TextureDeltaRayleighScatteringCrc(TextureDeltaRayleighScattering);
	constexpr StringCrc TextureDeltaMieScatteringCrc(TextureDeltaMieScattering);
	constexpr StringCrc TextureDeltaMultipleScatteringCrc(TextureDeltaMultipleScattering);
	constexpr StringCrc TextureDeltaIrradianceCrc(TextureDeltaIrradiance);
	constexpr StringCrc TextureDeltaScatteringDensityCrc(TextureDeltaScatteringDensity);
	constexpr StringCrc TextureIrradianceCrc(TextureIrradiance);
	constexpr StringCrc TextureScatteringCrc(TextureScattering);

	// In compute shader stage, use texture slot 15 - 9 to read and slot 0 -2 to write.
	const bgfx::ViewId viewId = static_cast<bgfx::ViewId>(GetViewID());

	// Compute Transmittance.
	bgfx::setImage(0, GetRenderContext()->GetTexture(TextureTransmittanceCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	GetRenderContext()->Dispatch(viewId, ProgramComputeTransmittanceCrc,
		TRANSMITTANCE_TEXTURE_WIDTH / 8U, TRANSMITTANCE_TEXTURE_HEIGHT / 8U, 1U);

	// Compute direct Irradiance.
	bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(TextureTransmittanceCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(0, GetRenderContext()->GetTexture(TextureDeltaIrradianceCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(1, GetRenderContext()->GetTexture(TextureIrradianceCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	GetRenderContext()->Dispatch(viewId, ProgramComputeDirectIrradianceCrc,
		IRRADIANCE_TEXTURE_WIDTH / 8U, IRRADIANCE_TEXTURE_HEIGHT / 8U, 1U);

	// Compute single Scattering.
	bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(TextureTransmittanceCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(0, GetRenderContext()->GetTexture(TextureDeltaRayleighScatteringCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(1, GetRenderContext()->GetTexture(TextureDeltaMieScatteringCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(2, GetRenderContext()->GetTexture(TextureScatteringCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	GetRenderContext()->Dispatch(viewId, ProgramComputeSingleScatteringCrc,
		SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);

	// Compute multiple Scattering.
	cd::Vec4f tmpOrder;
	for (uint16_t order = 2; order <= ScatteringOrders; ++order)
	{
		// 1. Compute Scattering Density.
		tmpOrder.x() = static_cast<float>(order);
		bgfx::setUniform(GetRenderContext()->GetUniform(NumScatteringOrdersCrc), &(tmpOrder.x()), 1);

		bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(TextureTransmittanceCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_SINGLE_RAYLEIGH_SCATTERING_SLOT, GetRenderContext()->GetTexture(TextureDeltaRayleighScatteringCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_SINGLE_MIE_SCATTERING_SLOT, GetRenderContext()->GetTexture(TextureDeltaMieScatteringCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_MULTIPLE_SCATTERING_SLOT, GetRenderContext()->GetTexture(TextureDeltaMultipleScatteringCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_IRRADIANCE_SLOT, GetRenderContext()->GetTexture(TextureDeltaIrradianceCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(0, GetRenderContext()->GetTexture(TextureDeltaScatteringDensityCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		GetRenderContext()->Dispatch(viewId, ProgramComputeScatteringDensityCrc,
			SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);

		// 2. Compute indirect Irradiance.
		tmpOrder.x() = static_cast<float>(order - 1);
		bgfx::setUniform(GetRenderContext()->GetUniform(NumScatteringOrdersCrc), &(tmpOrder.x()), 1);

		bgfx::setImage(ATM_SINGLE_RAYLEIGH_SCATTERING_SLOT, GetRenderContext()->GetTexture(TextureDeltaRayleighScatteringCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_SINGLE_MIE_SCATTERING_SLOT, GetRenderContext()->GetTexture(TextureDeltaMieScatteringCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_MULTIPLE_SCATTERING_SLOT, GetRenderContext()->GetTexture(TextureDeltaMultipleScatteringCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(0, GetRenderContext()->GetTexture(TextureDeltaIrradianceCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(1, GetRenderContext()->GetTexture(TextureIrradianceCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		GetRenderContext()->Dispatch(viewId, ProgramComputeIndirectIrradianceCrc,
			IRRADIANCE_TEXTURE_WIDTH / 8U, IRRADIANCE_TEXTURE_HEIGHT / 8U, 1U);

		// 3. Compute multiple Scattering.
		bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(TextureTransmittanceCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_SCATTERING_DENSITY, GetRenderContext()->GetTexture(TextureDeltaScatteringDensityCrc), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(0, GetRenderContext()->GetTexture(TextureDeltaMultipleScatteringCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(1, GetRenderContext()->GetTexture(TextureScatteringCrc), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		GetRenderContext()->Dispatch(viewId, ProgramComputeMultipleScatteringCrc,
			SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);
	}

	CD_ENGINE_TRACE("All compute shaders for precomputing atmospheric scattering texture dispatched.");
	CD_ENGINE_INFO("Atmospheric scattering orders : {0}", ScatteringOrders);
	
	auto skyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());
	skyComponent->SetATMTransmittanceCrc(TextureTransmittanceCrc);
	skyComponent->SetATMIrradianceCrc(TextureIrradianceCrc);
	skyComponent->SetATMScatteringCrc(TextureScatteringCrc);

	GetRenderContext()->DestoryTexture(TextureDeltaIrradianceCrc);
	GetRenderContext()->DestoryTexture(TextureDeltaRayleighScatteringCrc);
	GetRenderContext()->DestoryTexture(TextureDeltaMieScatteringCrc);
	GetRenderContext()->DestoryTexture(TextureDeltaScatteringDensityCrc);
	GetRenderContext()->DestoryTexture(TextureDeltaMultipleScatteringCrc);
}

}