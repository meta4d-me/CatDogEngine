#pragma once

#include "Renderer.h"
#include "Math/VectorDerived.hpp"

#include <bgfx/bgfx.h>

namespace engine
{

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
	cd::Vec4f m_uniformData;

	bool m_precomputeCache = false;

	// Skybox
	bgfx::VertexLayout m_vertexLayoutSkyBox;
	bgfx::VertexBufferHandle m_vbhSkybox;
	bgfx::IndexBufferHandle m_ibhSkybox;

	static constexpr cd::Vec3f ms_skyboxVertices[] = {
		cd::Vec3f(-1.0f,  1.0f,  1.0f),
		cd::Vec3f( 1.0f,  1.0f,  1.0f),
		cd::Vec3f(-1.0f, -1.0f,  1.0f),
		cd::Vec3f( 1.0f, -1.0f,  1.0f),
		cd::Vec3f(-1.0f,  1.0f, -1.0f),
		cd::Vec3f( 1.0f,  1.0f, -1.0f),
		cd::Vec3f(-1.0f, -1.0f, -1.0f),
		cd::Vec3f( 1.0f, -1.0f, -1.0f),
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
