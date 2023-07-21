#include "WorldRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "LightUniforms.h"
#include "Material/ShaderSchema.h"
#include "Math/Transform.hpp"
#include "RenderContext.h"
#include "Scene/Texture.h"
#include "U_PBR.sh"

//#include <format>

namespace engine
{

void WorldRenderer::Init()
{
	GetRenderContext()->CreateUniform("u_lightCountAndStride", bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform("u_lightParams", bgfx::UniformType::Vec4, LightUniform::VEC4_COUNT);

	GetRenderContext()->CreateUniform("s_texLUT", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateTexture("Textures/lut/ibl_brdf_lut.dds");

	GetRenderContext()->CreateUniform("s_texCube", bgfx::UniformType::Sampler);
	GetRenderContext()->CreateUniform("s_texCubeIrr", bgfx::UniformType::Sampler);
	uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
	GetRenderContext()->CreateTexture("Textures/skybox/bolonga_lod.dds", samplerFlags);
	GetRenderContext()->CreateTexture("Textures/skybox/bolonga_irr.dds", samplerFlags);

	GetRenderContext()->CreateUniform("u_cameraPos", bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform("u_albedoColor", bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform("u_emissiveColor", bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform("u_albedoUVOffsetAndScale", bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform("u_alphaCutOff", bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform("u_metallicRoughnessFactor", bgfx::UniformType::Vec4, 1);

	bgfx::setViewName(GetViewID(), "WorldRenderer");
}

void WorldRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void WorldRenderer::Render(float deltaTime)
{
	// TODO : Remove it. If every renderer need to submit camera related uniform, it should be done not inside Renderer class.
	const engine::CameraComponent* pCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
	const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();
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
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{pMeshComponent->GetVertexBuffer()});
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{pMeshComponent->GetIndexBuffer()});

		// Material
		for (const auto& [textureType, _] : pMaterialComponent->GetTextureResources())
		{
			if (const MaterialComponent::TextureInfo* pTextureInfo = pMaterialComponent->GetTextureInfo(textureType))
			{
				if (cd::MaterialTextureType::BaseColor == textureType)
				{
					constexpr StringCrc uvOffsetAndScale("u_albedoUVOffsetAndScale");
					cd::Vec4f uvOffsetAndScaleData(pTextureInfo->GetUVOffset().x(), pTextureInfo->GetUVOffset().y(),
						pTextureInfo->GetUVScale().x(), pTextureInfo->GetUVScale().y());
					GetRenderContext()->FillUniform(uvOffsetAndScale, &uvOffsetAndScaleData, 1);
				}

				bgfx::setTexture(pTextureInfo->slot, bgfx::UniformHandle{pTextureInfo->samplerHandle}, bgfx::TextureHandle{pTextureInfo->textureHandle});
			}
		}

		constexpr StringCrc lutSampler("s_texLUT");
		constexpr StringCrc lutTexture("Textures/lut/ibl_brdf_lut.dds");
		bgfx::setTexture(BRDF_LUT_SLOT, GetRenderContext()->GetUniform(lutSampler), GetRenderContext()->GetTexture(lutTexture));

		constexpr StringCrc useIBLCrc("USE_PBR_IBL");
		if (useIBLCrc == pMaterialComponent->GetUberShaderOption())
		{
			constexpr StringCrc cubeSampler("s_texCube");
			constexpr StringCrc cubeTexture("Textures/skybox/bolonga_lod.dds");
			bgfx::setTexture(IBL_RADIANCE_SLOT, GetRenderContext()->GetUniform(cubeSampler), GetRenderContext()->GetTexture(cubeTexture));

			constexpr StringCrc cubeIrrSampler("s_texCubeIrr");
			constexpr StringCrc cubeIrrTexture("Textures/skybox/bolonga_irr.dds");
			bgfx::setTexture(IBL_IRRADIANCE_SLOT, GetRenderContext()->GetUniform(cubeIrrSampler), GetRenderContext()->GetTexture(cubeIrrTexture));
		}

		// Submit uniform values : material settings
		constexpr StringCrc albedoColor("u_albedoColor");
		GetRenderContext()->FillUniform(albedoColor, pMaterialComponent->GetAlbedoColor().Begin(), 1);

		constexpr StringCrc mrFactor("u_metallicRoughnessFactor");
		cd::Vec4f metallicRoughnessFactorData(pMaterialComponent->GetMetallicFactor(), pMaterialComponent->GetRoughnessFactor(), 1.0f, 1.0f);
		GetRenderContext()->FillUniform(mrFactor, metallicRoughnessFactorData.Begin(), 1);

		constexpr StringCrc emissiveColor("u_emissiveColor");
		GetRenderContext()->FillUniform(emissiveColor, pMaterialComponent->GetEmissiveColor().Begin(), 1);

		// Submit uniform values : camera settings
		constexpr StringCrc cameraPos("u_cameraPos");
		GetRenderContext()->FillUniform(cameraPos, &cameraTransform.GetTranslation().x(), 1);

		// Submit uniform values : light settings
		auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();
		size_t lightEntityCount = lightEntities.size();
		constexpr engine::StringCrc lightCountAndStride("u_lightCountAndStride");
		static cd::Vec4f lightInfoData(0, LightUniform::LIGHT_STRIDE, 0.0f, 0.0f);
		lightInfoData.x() = static_cast<float>(lightEntityCount);
		GetRenderContext()->FillUniform(lightCountAndStride, lightInfoData.Begin(), 1);
		if (lightEntityCount > 0)
		{
			// Light component storage has continus memory address and layout.
			float* pLightDataBegin = reinterpret_cast<float*>(m_pCurrentSceneWorld->GetLightComponent(lightEntities[0]));
			constexpr engine::StringCrc lightParams("u_lightParams");
			GetRenderContext()->FillUniform(lightParams, pLightDataBegin, static_cast<uint16_t>(lightEntityCount * LightUniform::LIGHT_STRIDE));
		}

		//
		constexpr uint64_t defaultState = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		uint64_t state = defaultState;
		if (!pMaterialComponent->GetTwoSided())
		{
			state |= BGFX_STATE_CULL_CCW;
		}

		if (cd::BlendMode::Mask == pMaterialComponent->GetBlendMode())
		{
			constexpr StringCrc uvOffsetAndScale("u_alphaCutOff");
			GetRenderContext()->FillUniform(uvOffsetAndScale, &pMaterialComponent->GetAlphaCutOff(), 1);
		}

		bgfx::setState(state);

		bgfx::submit(GetViewID(), bgfx::ProgramHandle{pMaterialComponent->GetShadingProgram()});
	}
}

}