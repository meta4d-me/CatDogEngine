#include "WorldRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "fs_PBR_definitions.sh"
#include "LightUniforms.h"
#include "Material/ShaderSchema.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

#include <format>

namespace engine
{

void WorldRenderer::Init()
{
	m_pRenderContext->CreateUniform("u_lightParams", bgfx::UniformType::Vec4, LightUniform::VEC4_COUNT);
	m_pRenderContext->CreateUniform("s_texLUT", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateTexture("Textures/lut/ibl_brdf_lut.dds");

	m_pRenderContext->CreateUniform("s_texCube", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform("s_texCubeIrr", bgfx::UniformType::Sampler);
	uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
	m_pRenderContext->CreateTexture("Textures/skybox/bolonga_lod.dds", samplerFlags);
	m_pRenderContext->CreateTexture("Textures/skybox/bolonga_irr.dds", samplerFlags);

	m_pRenderContext->CreateUniform("u_cameraPos", bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateUniform("u_albedoColor", bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateUniform("u_emissiveColor", bgfx::UniformType::Vec4, 1);
	m_pRenderContext->CreateUniform("u_albedoUVOffsetAndScale", bgfx::UniformType::Vec4, 1);

	bgfx::setViewName(GetViewID(), "WorldRenderer");
}

void WorldRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void WorldRenderer::Render(float deltaTime)
{
	// TODO : Remove it. If every renderer need to submit camera related uniform, it should be done not inside Renderer class.
	const engine::CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
	for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
	{
		MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		if (!pMaterialComponent ||
			pMaterialComponent->GetMaterialType() != m_pCurrentSceneWorld->GetPBRMaterialType())
		{
			// TODO : improve this condition. As we want to skip some feature-specified entities to render.
			// For example, terrain/particle/...
			continue;
		}

		// No mesh attached?
		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		// SkinMesh
		if(m_pCurrentSceneWorld->GetAnimationComponent(entity))
		{
			continue;
		}

		// Transform
		if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		// Mesh
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle(pMeshComponent->GetVertexBuffer()));
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle(pMeshComponent->GetIndexBuffer()));

		// Material
		for (const auto& [textureType, textureInfo] : pMaterialComponent->GetTextureResources())
		{
			std::optional<MaterialComponent::TextureInfo> optTextureInfo = pMaterialComponent->GetTextureInfo(textureType);
			if (optTextureInfo.has_value())
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

		constexpr StringCrc albedoColor("u_albedoColor");
		m_pRenderContext->FillUniform(albedoColor, pMaterialComponent->GetAlbedoColor().Begin(), 1);

		constexpr StringCrc emissiveColor("u_emissiveColor");
		m_pRenderContext->FillUniform(emissiveColor, pMaterialComponent->GetEmissiveColor().Begin(), 1);

		constexpr StringCrc lutSampler("s_texLUT");
		constexpr StringCrc lutTexture("Textures/lut/ibl_brdf_lut.dds");
		bgfx::setTexture(LUT_SLOT, m_pRenderContext->GetUniform(lutSampler), m_pRenderContext->GetTexture(lutTexture));

		constexpr StringCrc useIBLCrc("USE_PBR_IBL");
		if (useIBLCrc == pMaterialComponent->GetUberShaderOption())
		{
			constexpr StringCrc cubeSampler("s_texCube");
			constexpr StringCrc cubeTexture("Textures/skybox/bolonga_lod.dds");
			bgfx::setTexture(IBL_ALBEDO_SLOT, m_pRenderContext->GetUniform(cubeSampler), m_pRenderContext->GetTexture(cubeTexture));

			constexpr StringCrc cubeIrrSampler("s_texCubeIrr");
			constexpr StringCrc cubeIrrTexture("Textures/skybox/bolonga_irr.dds");
			bgfx::setTexture(IBL_IRRADIANCE_SLOT, m_pRenderContext->GetUniform(cubeIrrSampler), m_pRenderContext->GetTexture(cubeIrrTexture));
		}

		//auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();
		//size_t lightEntityCount = lightEntities.size();
		//if (lightEntityCount > 0)
		//{
		//	// Light component storage has continus memory address and layout.
		//	float* pLightDataBegin = reinterpret_cast<float*>(m_pCurrentSceneWorld->GetLightComponent(lightEntities[0]));
		//	constexpr engine::StringCrc lightParams("u_lightParams");
		//	m_pRenderContext->FillUniform(lightParams, pLightDataBegin, static_cast<uint16_t>(lightEntityCount * LightUniform::LIGHT_STRIDE));
		//}

		constexpr StringCrc cameraPos("u_cameraPos");
		m_pRenderContext->FillUniform(cameraPos, &pCameraComponent->GetEye().x(), 1);

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		bgfx::submit(GetViewID(), bgfx::ProgramHandle(pMaterialComponent->GetShadingProgram()));
	}
}

}