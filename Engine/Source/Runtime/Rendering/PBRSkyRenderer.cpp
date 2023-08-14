#include "PBRSkyRenderer.h"

#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkyComponent.h"
#include "Log/Log.h"
#include "Math/Box.hpp"
#include "RenderContext.h"
#include "Scene/Mesh.h"
#include "Scene/VertexFormat.h"
#include "U_AtmophericScattering.sh"

namespace engine
{

namespace
{

constexpr const char* ProgramAtmosphericScatteringLUT    = "programAtmosphericScatteringLUT";
constexpr const char* ProgramSingleScatteringRayMarching = "programSingleScatteringRayMarching";

// Compute shaders
constexpr const char* ProgramComputeTransmittance        = "programComputeTransmittance";
constexpr const char* ProgramComputeDirectIrradiance     = "programComputeDirectIrradiance";
constexpr const char* ProgramComputeSingleScattering     = "programComputeSingleScattering";
constexpr const char* ProgramComputeScatteringDensity    = "programComputeScatteringDensity";
constexpr const char* ProgramComputeIndirectIrradiance   = "programComputeIndirectIrradiance";
constexpr const char* ProgramComputeMultipleScattering   = "programComputeMultipleScattering";

constexpr const char* TextureTransmittance               = "textureTransmittance";
constexpr const char* TextureIrradiance                  = "textureIrradiance";
constexpr const char* TextureScattering                  = "textureScattering";
constexpr const char* TextureDeltaIrradiance             = "textureDeltaIrradiance";
constexpr const char* TextureDeltaRayleighScattering     = "textureDeltaRayleighScattering";
constexpr const char* TextureDeltaMieScattering          = "textureDeltaMieScattering";
constexpr const char* TextureDeltaScatteringDensity      = "textureDeltaScatteringDensity";
constexpr const char* TextureDeltaMultipleScattering     = "textureDeltaMultipleScattering";

constexpr const char* LightDir                           = "u_LightDir";
constexpr const char* CameraPos                          = "u_cameraPos";
constexpr const char* NumScatteringOrders                = "u_numScatteringOrders";
										                 
constexpr uint64_t FlagTexture2D                         = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
constexpr uint64_t FlagTexture3D                         = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_UVW_CLAMP;
constexpr uint64_t StateRendering                        = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LEQUAL;
constexpr uint16_t ScatteringOrders                      = 6;

}

void PBRSkyRenderer::Init()
{
	SkyComponent* pSkyComponent = m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity());

	GetRenderContext()->CreateProgram(ProgramAtmosphericScatteringLUT, "vs_atmSkyBox.bin", "fs_PrecomputedAtmosphericScattering_LUT.bin");
	GetRenderContext()->CreateProgram(ProgramSingleScatteringRayMarching, "vs_atmSkyBox.bin", "fs_SingleScattering_RayMarching.bin");

	GetRenderContext()->CreateProgram(ProgramComputeTransmittance,      "cs_ComputeTransmittance.bin");
	GetRenderContext()->CreateProgram(ProgramComputeDirectIrradiance,   "cs_ComputeDirectIrradiance.bin");
	GetRenderContext()->CreateProgram(ProgramComputeSingleScattering,   "cs_ComputeSingleScattering.bin");
	GetRenderContext()->CreateProgram(ProgramComputeScatteringDensity,  "cs_ComputeScatteringDensity.bin");
	GetRenderContext()->CreateProgram(ProgramComputeIndirectIrradiance, "cs_ComputeIndirectIrradiance.bin");
	GetRenderContext()->CreateProgram(ProgramComputeMultipleScattering, "cs_ComputeMultipleScattering.bin");

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
	bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pMeshComponent->GetIndexBuffer() });
	bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pMeshComponent->GetVertexBuffer() });

	bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureTransmittance)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(ATM_IRRADIANCE_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureIrradiance)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(ATM_SCATTERING_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureScattering)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);

	// Unit: km
	// TODO : Use real camera component data.
	cd::Vec4f tmpCameraPos = cd::Vec4f(0.0f, 1.0f, -0.5f, 1.0f);
	bgfx::setUniform(GetRenderContext()->GetUniform(StringCrc(CameraPos)), &(tmpCameraPos.x()), 1);

	TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetSkyEntity());
	bgfx::setUniform(GetRenderContext()->GetUniform(StringCrc(LightDir)), &(pTransformComponent->GetTransform().GetTranslation()), 1);
	// Its better to use the rotation of TransformComponent to represent light direction.
	// As the rotate ui is difficult to control for now, we use the transform of TransformComponent to represent light direction.
	// cd::Vec3f tmpLightDir = pTransformComponent->GetTransform().GetRotation().ToEulerAngles();
	// bgfx::setUniform(GetRenderContext()->GetUniform(StringCrc(LightDir)), &(tmpLightDir.x()), 1);

	bgfx::setState(StateRendering);
	bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(StringCrc(ProgramAtmosphericScatteringLUT)));
}

bool PBRSkyRenderer::IsEnable() const
{
	return SkyType::AtmosphericScattering == m_pCurrentSceneWorld->GetSkyComponent(m_pCurrentSceneWorld->GetSkyEntity())->GetSkyType();
}

void PBRSkyRenderer::Precompute() const
{
	// In compute shader stage, use texture slot 15 - 9 to read and slot 0 -2 to write.
	const bgfx::ViewId viewId = static_cast<bgfx::ViewId>(GetViewID());

	// Compute Transmittance.
	bgfx::setImage(0, GetRenderContext()->GetTexture(StringCrc(TextureTransmittance)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	bgfx::dispatch(viewId, GetRenderContext()->GetProgram(StringCrc(ProgramComputeTransmittance)), TRANSMITTANCE_TEXTURE_WIDTH / 8U, TRANSMITTANCE_TEXTURE_HEIGHT / 8U, 1U);

	// Compute direct Irradiance.
	bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureTransmittance)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(0, GetRenderContext()->GetTexture(StringCrc(TextureDeltaIrradiance)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(1, GetRenderContext()->GetTexture(StringCrc(TextureIrradiance)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	bgfx::dispatch(viewId, GetRenderContext()->GetProgram(StringCrc(ProgramComputeDirectIrradiance)), IRRADIANCE_TEXTURE_WIDTH / 8U, IRRADIANCE_TEXTURE_HEIGHT / 8U, 1U);

	// Compute single Scattering.
	bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureTransmittance)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(0, GetRenderContext()->GetTexture(StringCrc(TextureDeltaRayleighScattering)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(1, GetRenderContext()->GetTexture(StringCrc(TextureDeltaMieScattering)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(2, GetRenderContext()->GetTexture(StringCrc(TextureScattering)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
	bgfx::dispatch(viewId, GetRenderContext()->GetProgram(StringCrc(ProgramComputeSingleScattering)), SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);

	// Compute multiple Scattering.
	cd::Vec4f tmpOrder;
	for (uint16_t order = 2; order <= ScatteringOrders; ++order)
	{

		// 1. Compute Scattering Density.
		tmpOrder.x() = static_cast<float>(order);
		bgfx::setUniform(GetRenderContext()->GetUniform(StringCrc(NumScatteringOrders)), &(tmpOrder.x()), 1);

		bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureTransmittance)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_SINGLE_RAYLEIGH_SCATTERING_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureDeltaRayleighScattering)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_SINGLE_MIE_SCATTERING_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureDeltaMieScattering)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_MULTIPLE_SCATTERING_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureDeltaMultipleScattering)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_IRRADIANCE_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureDeltaIrradiance)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(0, GetRenderContext()->GetTexture(StringCrc(TextureDeltaScatteringDensity)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::dispatch(viewId, GetRenderContext()->GetProgram(StringCrc(ProgramComputeScatteringDensity)), SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);

		// 2. Compute indirect Irradiance.
		tmpOrder.x() = static_cast<float>(order - 1);
		bgfx::setUniform(GetRenderContext()->GetUniform(StringCrc(NumScatteringOrders)), &(tmpOrder.x()), 1);

		bgfx::setImage(ATM_SINGLE_RAYLEIGH_SCATTERING_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureDeltaRayleighScattering)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_SINGLE_MIE_SCATTERING_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureDeltaMieScattering)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_MULTIPLE_SCATTERING_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureDeltaMultipleScattering)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(0, GetRenderContext()->GetTexture(StringCrc(TextureDeltaIrradiance)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(1, GetRenderContext()->GetTexture(StringCrc(TextureIrradiance)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::dispatch(viewId, GetRenderContext()->GetProgram(StringCrc(ProgramComputeIndirectIrradiance)), IRRADIANCE_TEXTURE_WIDTH / 8U, IRRADIANCE_TEXTURE_HEIGHT / 8U, 1U);

		// 3. Compute multiple Scattering.
		bgfx::setImage(ATM_TRANSMITTANCE_SLOT, GetRenderContext()->GetTexture(StringCrc(TextureTransmittance)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(ATM_SCATTERING_DENSITY, GetRenderContext()->GetTexture(StringCrc(TextureDeltaScatteringDensity)), 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(0, GetRenderContext()->GetTexture(StringCrc(TextureDeltaMultipleScattering)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(1, GetRenderContext()->GetTexture(StringCrc(TextureScattering)), 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::dispatch(viewId, GetRenderContext()->GetProgram(StringCrc(ProgramComputeMultipleScattering)), SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);
	}

	CD_ENGINE_TRACE("All compute shaders for precomputing atmospheric scattering texture dispatched.");
	CD_ENGINE_INFO("Atmospheric scattering orders : {0}", ScatteringOrders);
	
	SafeDestroyTexture(TextureDeltaIrradiance);
	SafeDestroyTexture(TextureDeltaRayleighScattering);
	SafeDestroyTexture(TextureDeltaMieScattering);
	SafeDestroyTexture(TextureDeltaScatteringDensity);
	SafeDestroyTexture(TextureDeltaMultipleScattering);
}

void PBRSkyRenderer::SafeDestroyTexture(const char* str) const
{
	bgfx::TextureHandle handle = GetRenderContext()->GetTexture(StringCrc(str));
	if (bgfx::isValid(handle))
	{
		bgfx::destroy(handle);
	}
}

}