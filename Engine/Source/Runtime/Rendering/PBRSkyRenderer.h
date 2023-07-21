#pragma once

#include "ECWorld/Entity.h"
#include "Renderer.h"
#include "Math/Vector.hpp"
#include "Scene/Mesh.h"

#include <bgfx/bgfx.h>
#include <ECWorld/SceneWorld.h>

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
	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

private:
	void Precompute();
	void ClearTextureSlots() const;
	void ReleaseTemporaryTextureResources();

private:
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
	std::vector<cd::Point> m_vertexBufferSkybox;
	std::vector<cd::Polygon> m_indexBufferSkybox;
	bgfx::VertexBufferHandle m_vbhSkybox;
	bgfx::IndexBufferHandle m_ibhSkybox;


	SceneWorld* m_pCurrentSceneWorld = nullptr;
	SkyComponent* m_pSkyComponent = nullptr;
};

}