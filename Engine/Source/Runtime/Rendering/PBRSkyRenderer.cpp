#include "PBRSkyRenderer.h"

#include "Log/Log.h"
#include "Math/Box.hpp"
#include "Math/MeshGenerator.h"
#include "RenderContext.h"
#include "Scene/Mesh.h"
#include "Scene/VertexFormat.h"
#include "U_AtmTextureSize.sh"

namespace engine
{

namespace
{

constexpr uint64_t FLAG_2DTEXTURE = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
constexpr uint64_t FLAG_3DTEXTURE = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_UVW_CLAMP;
constexpr uint64_t RENDERING_STATE = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LEQUAL;
	
constexpr uint16_t SCATTERING_ORDERS = 6;

}

PBRSkyRenderer::~PBRSkyRenderer() = default;

void PBRSkyRenderer::Init() {
	bgfx::ShaderHandle vsh_skyBox             = GetRenderContext()->CreateShader("vs_atmSkyBox.bin");
	bgfx::ShaderHandle fsh_multipleScattering = GetRenderContext()->CreateShader("fs_PrecomputedAtmosphericScattering_LUT.bin");
	bgfx::ShaderHandle fsh_singleScattering   = GetRenderContext()->CreateShader("fs_SingleScattering_RayMarching.bin");
	m_programAtmosphericScattering_LUT        = GetRenderContext()->CreateProgram("AtmosphericScattering", vsh_skyBox, fsh_multipleScattering);
	m_programSingleScattering_RayMarching     = GetRenderContext()->CreateProgram("AtmosphericScattering", vsh_skyBox, fsh_singleScattering);

	m_programComputeTransmittance      = GetRenderContext()->CreateProgram("ComputeTransmittance", "cs_ComputeTransmittance.bin");
	m_programComputeDirectIrradiance   = GetRenderContext()->CreateProgram("ComputeDirectIrradiance", "cs_ComputeDirectIrradiance.bin");
	m_programComputeSingleScattering   = GetRenderContext()->CreateProgram("ComputeSingleScattering", "cs_ComputeSingleScattering.bin");
	m_programComputeScatteringDensity  = GetRenderContext()->CreateProgram("ComputeScatteringDensity", "cs_ComputeScatteringDensity.bin");
	m_programComputeIndirectIrradiance = GetRenderContext()->CreateProgram("ComputeIndirectIrradiance", "cs_ComputeIndirectIrradiance.bin");
	m_programComputeMultipleScattering = GetRenderContext()->CreateProgram("ComputeMultipleScattering", "cs_ComputeMultipleScattering.bin");

	m_textureTransmittance = GetRenderContext()->CreateTexture("m_textureTransmittance",
		TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, 1, bgfx::TextureFormat::RGBA32F, FLAG_2DTEXTURE);
	m_textureIrradiance = GetRenderContext()->CreateTexture("m_textureIrradiance",
		IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 1, bgfx::TextureFormat::RGBA32F, FLAG_2DTEXTURE);
	m_textureDeltaIrradiance = GetRenderContext()->CreateTexture("m_textureDeltaIrradiance",
		IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 1, bgfx::TextureFormat::RGBA32F, FLAG_2DTEXTURE);
	m_textureDeltaRayleighScattering = GetRenderContext()->CreateTexture("m_textureDeltaRayleighScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, bgfx::TextureFormat::RGBA32F, FLAG_3DTEXTURE);
	m_textureDeltaMieScattering = GetRenderContext()->CreateTexture("m_textureDeltaMieScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, bgfx::TextureFormat::RGBA32F, FLAG_3DTEXTURE);
	m_textureScattering = GetRenderContext()->CreateTexture("m_textureScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, bgfx::TextureFormat::RGBA32F, FLAG_3DTEXTURE);
	m_textureDeltaScatteringDensity = GetRenderContext()->CreateTexture("m_textureDeltaScatteringDensity",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, bgfx::TextureFormat::RGBA32F, FLAG_3DTEXTURE);
	m_textureDeltaMultipleScattering = GetRenderContext()->CreateTexture("m_textureDeltaMultipleScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, bgfx::TextureFormat::RGBA32F, FLAG_3DTEXTURE);

	u_LightDir              = GetRenderContext()->CreateUniform("u_LightDir", bgfx::UniformType::Enum::Vec4, 1);
	u_cameraPos             = GetRenderContext()->CreateUniform("u_cameraPos", bgfx::UniformType::Enum::Vec4, 1);
	u_num_scattering_orders = GetRenderContext()->CreateUniform("u_num_scattering_orders", bgfx::UniformType::Enum::Vec4, 1);

	// Create vertex format and vertex/index buffer
	cd::VertexFormat vertexFormat;
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);
	
	constexpr StringCrc positionVertexLayout("PosistionOnly");
	GetRenderContext()->CreateVertexLayout(positionVertexLayout, vertexFormat.GetVertexLayout());

	cd::Box box(cd::Point(-1.0f), cd::Point(1.0f));
	std::optional<cd::Mesh> optMesh = cd::MeshGenerator::Generate(box, vertexFormat, false);
	assert(optMesh.has_value());

	const cd::Mesh& meshData = optMesh.value();
	m_vertexBufferSkybox = meshData.GetVertexPositions();
	m_indexBufferSkybox = meshData.GetPolygons();
	m_vbhSkybox = bgfx::createVertexBuffer(bgfx::makeRef(m_vertexBufferSkybox.data(), static_cast<uint32_t>(m_vertexBufferSkybox.size() * sizeof(cd::Point))), GetRenderContext()->GetVertexLayout(positionVertexLayout));
	m_ibhSkybox = bgfx::createIndexBuffer(bgfx::makeRef(m_indexBufferSkybox.data(), static_cast<uint32_t>(m_indexBufferSkybox.size() * sizeof(uint32_t) * 3)), BGFX_BUFFER_INDEX32);

	bgfx::setViewName(GetViewID(), "PBRSkyRenderer");
}

void PBRSkyRenderer::UpdateView(const float *pViewMatrix, const float *pProjectionMatrix) {
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
	m_uniformData = cd::Vec4f(0.0f, 1.0f, -0.5f, 1.0f);
	bgfx::setUniform(u_cameraPos, &m_uniformData.x(), 1);

	Entity entity = m_pCurrentSceneWorld->GetSkyEntity();
	TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity);
	bgfx::setUniform(u_LightDir, &pTransformComponent->GetTransform().GetTranslation(), 1);

	bgfx::setState(RENDERING_STATE);
	bgfx::submit(GetViewID(), m_programAtmosphericScattering_LUT);
}

void PBRSkyRenderer::Precompute() {
	if (!m_precomputeCache) {
		m_precomputeCache = true;
		// texture slot 0 - 7 to read, slot 8 - 15 to write.
		const uint16_t viewId = GetViewID();

		// Compute Transmittance.
		bgfx::setImage(8, m_textureTransmittance, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::dispatch(viewId, m_programComputeTransmittance, TRANSMITTANCE_TEXTURE_WIDTH / 8U, TRANSMITTANCE_TEXTURE_HEIGHT / 8U, 1U);

		// Compute direct Irradiance.
		bgfx::setImage(0, m_textureTransmittance, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(8, m_textureDeltaIrradiance, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(9, m_textureIrradiance, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::dispatch(viewId, m_programComputeDirectIrradiance, IRRADIANCE_TEXTURE_WIDTH / 8U, IRRADIANCE_TEXTURE_HEIGHT / 8U, 1U);

		// Compute single Scattering.
		bgfx::setImage(0, m_textureTransmittance, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(8, m_textureDeltaRayleighScattering, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(9, m_textureDeltaMieScattering, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::setImage(10, m_textureScattering, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
		bgfx::dispatch(viewId, m_programComputeSingleScattering, SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);

		// Compute multiple Scattering.
		for (uint16_t order = 2; order <= SCATTERING_ORDERS; ++order) {

			// 1. Compute Scattering Density.
			m_uniformData.x() = static_cast<float>(order);
			bgfx::setUniform(u_num_scattering_orders, &m_uniformData.x(), 1);

			bgfx::setImage(0, m_textureTransmittance, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(1, m_textureDeltaRayleighScattering, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(2, m_textureDeltaMieScattering, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(3, m_textureDeltaMultipleScattering, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(5, m_textureDeltaIrradiance, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(8, m_textureDeltaScatteringDensity, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
			bgfx::dispatch(viewId, m_programComputeScatteringDensity, SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);

			// 2. Compute indirect Irradiance.
			m_uniformData.x() = static_cast<float>(order - uint16_t(1));
			bgfx::setUniform(u_num_scattering_orders, &m_uniformData.x(), 1);

			bgfx::setImage(1, m_textureDeltaRayleighScattering, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(2, m_textureDeltaMieScattering, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(3, m_textureDeltaMultipleScattering, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(8, m_textureDeltaIrradiance, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(9, m_textureIrradiance, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
			bgfx::dispatch(viewId, m_programComputeIndirectIrradiance, IRRADIANCE_TEXTURE_WIDTH / 8U, IRRADIANCE_TEXTURE_HEIGHT / 8U, 1U);


			// 3. Compute multiple Scattering.
			bgfx::setImage(0, m_textureTransmittance, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(4, m_textureDeltaScatteringDensity, 0, bgfx::Access::Read, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(8, m_textureDeltaMultipleScattering, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
			bgfx::setImage(9, m_textureScattering, 0, bgfx::Access::Write, bgfx::TextureFormat::RGBA32F);
			bgfx::dispatch(viewId, m_programComputeMultipleScattering, SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);
		}
		CD_ENGINE_TRACE("All compute shaders for precomputing atmospheric scattering texture dispatched.");
		CD_ENGINE_TRACE("Scattering Orders : {0}", SCATTERING_ORDERS);
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
	auto SafeDestroy = [](bgfx::TextureHandle &_handle) {
		if (bgfx::isValid(_handle)) {
			bgfx::destroy(_handle);
			_handle = BGFX_INVALID_HANDLE;
		}
	};
	SafeDestroy(m_textureDeltaIrradiance);
	SafeDestroy(m_textureDeltaRayleighScattering);
	SafeDestroy(m_textureDeltaMieScattering);
	SafeDestroy(m_textureDeltaScatteringDensity);
	SafeDestroy(m_textureDeltaMultipleScattering);
}

} // namespace engine
