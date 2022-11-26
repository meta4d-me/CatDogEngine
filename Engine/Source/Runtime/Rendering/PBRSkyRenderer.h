#pragma once

#include "Renderer.h"
#include "Math/VectorDerived.hpp"

namespace engine
{

static constexpr uint16_t TRANSMITTANCE_TEXTURE_WIDTH = 256;
static constexpr uint16_t TRANSMITTANCE_TEXTURE_HEIGHT = 64;

static constexpr uint16_t SCATTERING_TEXTURE_R_SIZE = 32;
static constexpr uint16_t SCATTERING_TEXTURE_MU_SIZE = 128;
static constexpr uint16_t SCATTERING_TEXTURE_MU_S_SIZE = 32;
static constexpr uint16_t SCATTERING_TEXTURE_NU_SIZE = 8;

static constexpr uint16_t SCATTERING_TEXTURE_WIDTH = SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE;
static constexpr uint16_t SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;
static constexpr uint16_t SCATTERING_TEXTURE_DEPTH = SCATTERING_TEXTURE_R_SIZE;

static constexpr uint16_t IRRADIANCE_TEXTURE_WIDTH = 64;
static constexpr uint16_t IRRADIANCE_TEXTURE_HEIGHT = 16;

class PBRSkyRenderer final : public Renderer
{
public:
	using Renderer::Renderer;
	virtual ~PBRSkyRenderer();

	virtual void Init() override;
	virtual void UpdateView(const float *pViewMatrix, const float *pProjectionMatrix) override;
	virtual void Render(float deltaTime) override;

private:
	void Precompute();
	void ClearTextureSlots() const;
	void ReleaseTemporaryTextureResources();

	bgfx::ProgramHandle m_programSingleScattering_RayMarching;
	bgfx::ProgramHandle m_programAtmosphericScattering_LUT;

	// Compute shaders
	bgfx::ProgramHandle m_programComputeTransmittance;
	bgfx::ProgramHandle m_programComputeDirectIrradiance;
	bgfx::ProgramHandle m_programComputeSingleScattering;
	bgfx::ProgramHandle m_programComputeScatteringDensity;
	bgfx::ProgramHandle m_programComputeIndirectIrradiance;
	bgfx::ProgramHandle m_programComputeMultipleScattering;

	// Precompute textures
	bgfx::TextureHandle m_textureTransmittance;
	bgfx::TextureHandle m_textureIrradiance;
	bgfx::TextureHandle m_textureScattering;
	bgfx::TextureHandle m_textureDeltaIrradiance;
	bgfx::TextureHandle m_textureDeltaRayleighScattering;
	bgfx::TextureHandle m_textureDeltaMieScattering;
	bgfx::TextureHandle m_textureDeltaScatteringDensity;
	bgfx::TextureHandle m_textureDeltaMultipleScattering;

	// Uniforms
	bgfx::UniformHandle u_num_scattering_orders;
	bgfx::UniformHandle u_cameraPos;
	bgfx::UniformHandle u_LightDir;
	cdtools::Vec4f m_uniformData;

	bool m_precomputeCache = false;

	// Skybox
	bgfx::VertexLayout m_vertexLayoutSkyBox;
	bgfx::VertexBufferHandle m_vbhSkybox;
	bgfx::IndexBufferHandle m_ibhSkybox;

	static constexpr cdtools::Vec3f ms_skyboxVertices[] = {
		cdtools::Vec3f(-1.0f,  1.0f,  1.0f),
		cdtools::Vec3f( 1.0f,  1.0f,  1.0f),
		cdtools::Vec3f(-1.0f, -1.0f,  1.0f),
		cdtools::Vec3f( 1.0f, -1.0f,  1.0f),
		cdtools::Vec3f(-1.0f,  1.0f, -1.0f),
		cdtools::Vec3f( 1.0f,  1.0f, -1.0f),
		cdtools::Vec3f(-1.0f, -1.0f, -1.0f),
		cdtools::Vec3f( 1.0f, -1.0f, -1.0f),
	};

	static constexpr uint16_t ms_skyBoxIndeces[] = {
		// CCW
		0, 1, 2, // 0
		1, 3, 2,
		4, 6, 5, // 2
		5, 6, 7,
		0, 2, 4, // 4
		4, 2, 6,
		1, 5, 3, // 6
		5, 7, 3,
		0, 4, 1, // 8
		4, 5, 1,
		2, 3, 6, // 10
		6, 3, 7,
	};
};

} // namespace engine
