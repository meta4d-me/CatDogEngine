#include "PBRSkyRenderer.h"

#include "GBuffer.h"
#include "RenderContext.h"
#include "SwapChain.h"

#include <bx/math.h>

namespace engine
{

PBRSkyRenderer::~PBRSkyRenderer() {};

void PBRSkyRenderer::Init() {
	bgfx::ShaderHandle vsh_skyBox             = m_pRenderContext->CreateShader("vs_atmSkyBox.bin");
	bgfx::ShaderHandle fsh_multipleScattering = m_pRenderContext->CreateShader("fs_PrecomputedAtmosphericScattering_LUT.bin");
	bgfx::ShaderHandle fsh_singleScattering   = m_pRenderContext->CreateShader("fs_SingleScattering_RayMarching.bin");
	m_programAtmosphericScattering_LUT        = m_pRenderContext->CreateProgram("AtmosphericScattering", vsh_skyBox, fsh_multipleScattering);
	m_programSingleScattering_RayMarching     = m_pRenderContext->CreateProgram("AtmosphericScattering", vsh_skyBox, fsh_singleScattering);

	m_programComputeTransmittance      = m_pRenderContext->CreateProgram("ComputeTransmittance", "cs_ComputeTransmittance.bin");
	m_programComputeDirectIrradiance   = m_pRenderContext->CreateProgram("ComputeDirectIrradiance", "cs_ComputeDirectIrradiance.bin");
	m_programComputeSingleScattering   = m_pRenderContext->CreateProgram("ComputeSingleScattering", "cs_ComputeSingleScattering.bin");
	m_programComputeScatteringDensity  = m_pRenderContext->CreateProgram("ComputeScatteringDensity", "cs_ComputeScatteringDensity.bin");
	m_programComputeIndirectIrradiance = m_pRenderContext->CreateProgram("ComputeIndirectIrradiance", "cs_ComputeIndirectIrradiance.bin");
	m_programComputeMultipleScattering = m_pRenderContext->CreateProgram("ComputeMultipleScattering", "cs_ComputeMultipleScattering.bin");

	static constexpr uint64_t FLAG2D = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	static constexpr uint64_t FLAG3D = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_UVW_CLAMP;
	m_textureTransmittance = m_pRenderContext->CreateTexture("m_textureTransmittance",
		TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, FLAG2D);
	m_textureIrradiance = m_pRenderContext->CreateTexture("m_textureIrradiance",
		IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, FLAG2D);
	m_textureDeltaIrradiance = m_pRenderContext->CreateTexture("m_textureDeltaIrradiance",
		IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, FLAG2D);
	m_textureDeltaRayleighScattering = m_pRenderContext->CreateTexture("m_textureDeltaRayleighScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG3D);
	m_textureDeltaMieScattering = m_pRenderContext->CreateTexture("m_textureDeltaMieScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG3D);
	m_textureScattering = m_pRenderContext->CreateTexture("m_textureScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG3D);
	m_textureDeltaScatteringDensity = m_pRenderContext->CreateTexture("m_textureDeltaScatteringDensity",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG3D);
	m_textureDeltaMultipleScattering = m_pRenderContext->CreateTexture("m_textureDeltaMultipleScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG3D);

	u_num_scattering_orders = m_pRenderContext->CreateUniform("u_num_scattering_orders", bgfx::UniformType::Enum::Vec4, 1);
	u_cameraPos             = m_pRenderContext->CreateUniform("u_cameraPos", bgfx::UniformType::Enum::Vec4, 1);
	u_LightDir              = m_pRenderContext->CreateUniform("u_LightDir", bgfx::UniformType::Enum::Vec4, 1);

	m_vertexLayoutSkyBox.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
	m_vbhSkybox = bgfx::createVertexBuffer(bgfx::makeRef(ms_skyboxVertices, sizeof(ms_skyboxVertices)), m_vertexLayoutSkyBox);
	m_ibhSkybox = bgfx::createIndexBuffer(bgfx::makeRef(ms_skyBoxIndeces, sizeof(ms_skyBoxIndeces)));
}

void PBRSkyRenderer::UpdateView(const float *pViewMatrix, const float *pProjectionMatrix) {
	// For sky box.
	float pView[16];
	std::memcpy(pView, pViewMatrix, 12 * sizeof(float));
	pView[12] = pView[13] = pView[14] = 0.0f;
	pView[15] = 1.0f;

	bgfx::setViewFrameBuffer(GetViewID(), *m_pGBuffer->GetFrameBuffer());
	bgfx::setViewRect(GetViewID(), 0, 0, m_pGBuffer->GetWidth(), m_pGBuffer->GetHeight());
	bgfx::setViewTransform(GetViewID(), pView, pProjectionMatrix);
	bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
}

void PBRSkyRenderer::Render(float deltaTime) {
	Precompute();

	// Mesh
	bgfx::setVertexBuffer(0, m_vbhSkybox);
	bgfx::setIndexBuffer(m_ibhSkybox);

	// Texture
	bgfx::setImage(0, m_textureTransmittance, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(5, m_textureIrradiance, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	bgfx::setImage(6, m_textureScattering, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);

	// Uniform, temporary code, unit: km
	m_uniformData = cdtools::Vec4f(0.0f, 1.0, -0.5, 1.0f);
	bgfx::setUniform(u_cameraPos, &m_uniformData.x(), 1);
	m_uniformData = cdtools::Vec4f(0.0f, -1.0, -1.0, 0.0f);
	bgfx::setUniform(u_LightDir, &m_uniformData.x(), 1);

	// State
	static constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LEQUAL;
	bgfx::setState(state);

	bgfx::submit(GetViewID(), m_programAtmosphericScattering_LUT);
}

void PBRSkyRenderer::Precompute() {
	if (!m_precomputeCache) {
		m_precomputeCache = true;
		// texture slot 0 - 7 to read, slot 8 - 15 to write.

		using bgfx::Access::Read;
		using bgfx::Access::Write;
		using bgfx::TextureFormat::RGBA32F;
		const uint16_t viewId = GetViewID();

		// Compute Transmittance.
		bgfx::setImage(8, m_textureTransmittance, 0, Write, RGBA32F);
		bgfx::dispatch(viewId, m_programComputeTransmittance, TRANSMITTANCE_TEXTURE_WIDTH / 8, TRANSMITTANCE_TEXTURE_HEIGHT / 8, 1);

		// Compute direct Irradiance.
		bgfx::setImage(0, m_textureTransmittance, 0, Read, RGBA32F);
		bgfx::setImage(8, m_textureDeltaIrradiance, 0, Write, RGBA32F);
		bgfx::setImage(9, m_textureIrradiance, 0, Write, RGBA32F);
		bgfx::dispatch(viewId, m_programComputeDirectIrradiance, IRRADIANCE_TEXTURE_WIDTH / 8, IRRADIANCE_TEXTURE_HEIGHT / 8, 1);

		// Compute single Scattering.
		bgfx::setImage(0, m_textureTransmittance, 0, Read, RGBA32F);
		bgfx::setImage(8, m_textureDeltaRayleighScattering, 0, Write, RGBA32F);
		bgfx::setImage(9, m_textureDeltaMieScattering, 0, Write, RGBA32F);
		bgfx::setImage(10, m_textureScattering, 0, Write, RGBA32F);
		bgfx::dispatch(viewId, m_programComputeSingleScattering, SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);

		// Compute multiple Scattering.
		static constexpr uint16_t NUM_SCATTERING_ORDERS = 6;
		for (uint16_t order = 2; order <= NUM_SCATTERING_ORDERS; ++order) {

			// 1. Compute Scattering Density.
			m_uniformData.x() = static_cast<float>(order);
			bgfx::setUniform(u_num_scattering_orders, &m_uniformData.x(), 1);

			bgfx::setImage(0, m_textureTransmittance, 0, Read, RGBA32F);
			bgfx::setImage(1, m_textureDeltaRayleighScattering, 0, Read, RGBA32F);
			bgfx::setImage(2, m_textureDeltaMieScattering, 0, Read, RGBA32F);
			bgfx::setImage(3, m_textureDeltaMultipleScattering, 0, Read, RGBA32F);
			bgfx::setImage(5, m_textureDeltaIrradiance, 0, Read, RGBA32F);
			bgfx::setImage(8, m_textureDeltaScatteringDensity, 0, Write, RGBA32F);
			bgfx::dispatch(viewId, m_programComputeScatteringDensity, SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);

			// 2. Compute indirect Irradiance.
			m_uniformData.x() = static_cast<float>(order - uint16_t(1));
			bgfx::setUniform(u_num_scattering_orders, &m_uniformData.x(), 1);

			bgfx::setImage(1, m_textureDeltaRayleighScattering, 0, Read, RGBA32F);
			bgfx::setImage(2, m_textureDeltaMieScattering, 0, Read, RGBA32F);
			bgfx::setImage(3, m_textureDeltaMultipleScattering, 0, Read, RGBA32F);
			bgfx::setImage(8, m_textureDeltaIrradiance, 0, Write, RGBA32F);
			bgfx::setImage(9, m_textureIrradiance, 0, Write, RGBA32F);
			bgfx::dispatch(viewId, m_programComputeIndirectIrradiance, IRRADIANCE_TEXTURE_WIDTH / 8, IRRADIANCE_TEXTURE_HEIGHT / 8, 1);


			// 3. Compute multiple Scattering.
			bgfx::setImage(0, m_textureTransmittance, 0, Read, RGBA32F);
			bgfx::setImage(4, m_textureDeltaScatteringDensity, 0, Read, RGBA32F);
			bgfx::setImage(8, m_textureDeltaMultipleScattering, 0, Write, RGBA32F);
			bgfx::setImage(9, m_textureScattering, 0, Write, RGBA32F);
			bgfx::dispatch(viewId, m_programComputeMultipleScattering, SCATTERING_TEXTURE_WIDTH / 8, SCATTERING_TEXTURE_HEIGHT / 8, SCATTERING_TEXTURE_DEPTH / 8);
		}
		printf("\nAll compute shader for precompute atmospheric scattering texture dispatched.\nScattering Order : %d\n", NUM_SCATTERING_ORDERS);
		ClearTextureSlots();
		ReleaseTemporaryTextureResources();
	}
}

void PBRSkyRenderer::ClearTextureSlots() const {
	for (uint8_t i = 0; i < 16; ++i) {
		bgfx::setImage(i, BGFX_INVALID_HANDLE, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
	}
}

void PBRSkyRenderer::ReleaseTemporaryTextureResources() {
	auto SafeDelete = [](bgfx::TextureHandle &_handle) {
		if (bgfx::isValid(_handle)) {
			bgfx::destroy(_handle);
			_handle = BGFX_INVALID_HANDLE;
		}
	};
	SafeDelete(m_textureDeltaIrradiance);
	SafeDelete(m_textureDeltaRayleighScattering);
	SafeDelete(m_textureDeltaMieScattering);
	SafeDelete(m_textureDeltaScatteringDensity);
	SafeDelete(m_textureDeltaMultipleScattering);
}

} // namespace engine
