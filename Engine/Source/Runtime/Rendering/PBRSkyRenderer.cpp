#include "PBRSkyRenderer.h"

#include "Math/Box.hpp"
#include "Math/MeshGenerator.h"
#include "RenderContext.h"
#include "Scene/Mesh.h"
#include "Scene/VertexFormat.h"
#include "Shaders/UniformDefines/AtmosphericScatteringTextureSize.sh"

#include <bx/math.h>

namespace engine
{

namespace
{
	constexpr uint64_t FLAG_2DTEXTURE = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	constexpr uint64_t FLAG_3DTEXTURE = BGFX_TEXTURE_COMPUTE_WRITE | BGFX_SAMPLER_UVW_CLAMP;
	constexpr uint64_t RENDERING_STATE = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LEQUAL;
	
	constexpr uint16_t SCATTERING_ORDERS = 6;
}

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


	m_textureTransmittance = m_pRenderContext->CreateTexture("m_textureTransmittance",
		TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, FLAG_2DTEXTURE);
	m_textureIrradiance = m_pRenderContext->CreateTexture("m_textureIrradiance",
		IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, FLAG_2DTEXTURE);
	m_textureDeltaIrradiance = m_pRenderContext->CreateTexture("m_textureDeltaIrradiance",
		IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, FLAG_2DTEXTURE);
	m_textureDeltaRayleighScattering = m_pRenderContext->CreateTexture("m_textureDeltaRayleighScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG_3DTEXTURE);
	m_textureDeltaMieScattering = m_pRenderContext->CreateTexture("m_textureDeltaMieScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG_3DTEXTURE);
	m_textureScattering = m_pRenderContext->CreateTexture("m_textureScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG_3DTEXTURE);
	m_textureDeltaScatteringDensity = m_pRenderContext->CreateTexture("m_textureDeltaScatteringDensity",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG_3DTEXTURE);
	m_textureDeltaMultipleScattering = m_pRenderContext->CreateTexture("m_textureDeltaMultipleScattering",
		SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, FLAG_3DTEXTURE);

	u_LightDir              = m_pRenderContext->CreateUniform("u_LightDir", bgfx::UniformType::Enum::Vec4, 1);
	u_cameraPos             = m_pRenderContext->CreateUniform("u_cameraPos", bgfx::UniformType::Enum::Vec4, 1);
	u_num_scattering_orders = m_pRenderContext->CreateUniform("u_num_scattering_orders", bgfx::UniformType::Enum::Vec4, 1);

	// Create vertex format and vertex/index buffer
	cd::VertexFormat vertexFormat;
	vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);
	m_vertexLayoutSkyBox.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();

	cd::Box box(cd::Point(-1.0f), cd::Point(1.0f));
	std::optional<cd::Mesh> optMesh = cd::MeshGenerator::Generate(box, vertexFormat);
	assert(optMesh.has_value());

	const cd::Mesh& meshData = optMesh.value();
	static std::vector<cd::Point> vertexBufferData = meshData.GetVertexPositions();
	static std::vector<cd::Mesh::Polygon> indexBufferData = meshData.GetPolygons();
	m_vbhSkybox = bgfx::createVertexBuffer(bgfx::makeRef(vertexBufferData.data(), static_cast<uint32_t>(vertexBufferData.size() * sizeof(cd::Point))), m_vertexLayoutSkyBox);
	m_ibhSkybox = bgfx::createIndexBuffer(bgfx::makeRef(indexBufferData.data(), static_cast<uint32_t>(indexBufferData.size() * sizeof(uint32_t) * 3)), BGFX_BUFFER_INDEX32);

	bgfx::setViewName(GetViewID(), "PBRSkyRenderer");
}

void PBRSkyRenderer::UpdateView(const float *pViewMatrix, const float *pProjectionMatrix) {
	// We want the skybox to be centered around the player
	// so that no matter how far the player moves, the skybox won't get any closer.
	// Remove the translation part of the view matrix
	// so only rotation will affect the skybox's position vectors.
	float pView[16];
	std::memcpy(pView, pViewMatrix, 12 * sizeof(float));
	pView[12] = pView[13] = pView[14] = 0.0f;
	pView[15] = 1.0f;

	bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());
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
	m_uniformData = cd::Vec4f(0.0f, 1.0f, -0.5f, 1.0f);
	bgfx::setUniform(u_cameraPos, &m_uniformData.x(), 1);
	m_uniformData = cd::Vec4f(0.0f, -1.0f, -1.0f, 0.0f);
	bgfx::setUniform(u_LightDir, &m_uniformData.x(), 1);

	bgfx::setState(RENDERING_STATE);
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
		bgfx::dispatch(viewId, m_programComputeTransmittance, TRANSMITTANCE_TEXTURE_WIDTH / 8U, TRANSMITTANCE_TEXTURE_HEIGHT / 8U, 1U);

		// Compute direct Irradiance.
		bgfx::setImage(0, m_textureTransmittance, 0, Read, RGBA32F);
		bgfx::setImage(8, m_textureDeltaIrradiance, 0, Write, RGBA32F);
		bgfx::setImage(9, m_textureIrradiance, 0, Write, RGBA32F);
		bgfx::dispatch(viewId, m_programComputeDirectIrradiance, IRRADIANCE_TEXTURE_WIDTH / 8U, IRRADIANCE_TEXTURE_HEIGHT / 8U, 1U);

		// Compute single Scattering.
		bgfx::setImage(0, m_textureTransmittance, 0, Read, RGBA32F);
		bgfx::setImage(8, m_textureDeltaRayleighScattering, 0, Write, RGBA32F);
		bgfx::setImage(9, m_textureDeltaMieScattering, 0, Write, RGBA32F);
		bgfx::setImage(10, m_textureScattering, 0, Write, RGBA32F);
		bgfx::dispatch(viewId, m_programComputeSingleScattering, SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);

		// Compute multiple Scattering.
		for (uint16_t order = 2; order <= SCATTERING_ORDERS; ++order) {

			// 1. Compute Scattering Density.
			m_uniformData.x() = static_cast<float>(order);
			bgfx::setUniform(u_num_scattering_orders, &m_uniformData.x(), 1);

			bgfx::setImage(0, m_textureTransmittance, 0, Read, RGBA32F);
			bgfx::setImage(1, m_textureDeltaRayleighScattering, 0, Read, RGBA32F);
			bgfx::setImage(2, m_textureDeltaMieScattering, 0, Read, RGBA32F);
			bgfx::setImage(3, m_textureDeltaMultipleScattering, 0, Read, RGBA32F);
			bgfx::setImage(5, m_textureDeltaIrradiance, 0, Read, RGBA32F);
			bgfx::setImage(8, m_textureDeltaScatteringDensity, 0, Write, RGBA32F);
			bgfx::dispatch(viewId, m_programComputeScatteringDensity, SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);

			// 2. Compute indirect Irradiance.
			m_uniformData.x() = static_cast<float>(order - uint16_t(1));
			bgfx::setUniform(u_num_scattering_orders, &m_uniformData.x(), 1);

			bgfx::setImage(1, m_textureDeltaRayleighScattering, 0, Read, RGBA32F);
			bgfx::setImage(2, m_textureDeltaMieScattering, 0, Read, RGBA32F);
			bgfx::setImage(3, m_textureDeltaMultipleScattering, 0, Read, RGBA32F);
			bgfx::setImage(8, m_textureDeltaIrradiance, 0, Write, RGBA32F);
			bgfx::setImage(9, m_textureIrradiance, 0, Write, RGBA32F);
			bgfx::dispatch(viewId, m_programComputeIndirectIrradiance, IRRADIANCE_TEXTURE_WIDTH / 8U, IRRADIANCE_TEXTURE_HEIGHT / 8U, 1U);


			// 3. Compute multiple Scattering.
			bgfx::setImage(0, m_textureTransmittance, 0, Read, RGBA32F);
			bgfx::setImage(4, m_textureDeltaScatteringDensity, 0, Read, RGBA32F);
			bgfx::setImage(8, m_textureDeltaMultipleScattering, 0, Write, RGBA32F);
			bgfx::setImage(9, m_textureScattering, 0, Write, RGBA32F);
			bgfx::dispatch(viewId, m_programComputeMultipleScattering, SCATTERING_TEXTURE_WIDTH / 8U, SCATTERING_TEXTURE_HEIGHT / 8U, SCATTERING_TEXTURE_DEPTH / 8U);
		}
		printf("\nAll compute shaders for precomputing atmospheric scattering texture dispatched.\nScattering Orders : %d\n", SCATTERING_ORDERS);
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
