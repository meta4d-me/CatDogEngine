#include "WorldRenderer.h"

#include "ECWorld/MaterialComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ECWorld/World.h"
#include "Display/Camera.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

#include <format>

namespace engine
{

void WorldRenderer::Init()
{
	bgfx::ShaderHandle vsh = m_pRenderContext->CreateShader("vs_PBR.bin");
	m_programPBR = m_pRenderContext->CreateProgram("PBR", vsh, m_pRenderContext->CreateShader("fs_PBR.bin"));
	m_programPBR_AO = m_pRenderContext->CreateProgram("PBR_AO", vsh, m_pRenderContext->CreateShader("fs_PBR_AO.bin"));

	m_pRenderContext->CreateUniform("s_texCube", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform("s_texCubeIrr", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform("s_texLUT", bgfx::UniformType::Sampler);
	uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
	m_pRenderContext->CreateTexture("skybox/bolonga_lod.dds", samplerFlags);
	m_pRenderContext->CreateTexture("skybox/bolonga_irr.dds", samplerFlags);
	m_pRenderContext->CreateTexture("ibl_brdf_lut.dds");

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
	auto pMeshStorage = m_pCurrentWorld->GetComponents<StaticMeshComponent>();
	auto pMaterialStorage = m_pCurrentWorld->GetComponents<MaterialComponent>();
	auto pTransformStorage = m_pCurrentWorld->GetComponents<TransformComponent>();

	for (Entity entity : *m_pMeshEntites)
	{
		StaticMeshComponent* pMeshComponent = pMeshStorage->GetComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		TransformComponent* pTransformComponent = pTransformStorage->GetComponent(entity);
		if (pTransformComponent)
		{
			// TODO : test
			//pTransformComponent->SetTranslation(pTransformComponent->GetTranslation() + cd::Vec3f(0.0f, 0.0f, deltaTime * 0.1));
			//pTransformComponent->SetRotation(cd::Quaternion::FromAxisAngle(cd::Vec3f(0.0f, 0.0f, 1.0f), 0.0f));
			//pTransformComponent->SetScale(cd::Vec3f(0.2f, 0.2f, 0.2f));

			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Transpose().Begin());
		}

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle(pMeshComponent->GetVertexBuffer()));
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle(pMeshComponent->GetIndexBuffer()));

		bgfx::ProgramHandle shadingProgram(BGFX_INVALID_HANDLE);
		MaterialComponent* pMaterialComponent = pMaterialStorage->GetComponent(entity);
		if (pMaterialComponent)
		{
			shadingProgram.idx = pMaterialComponent->GetShadingProgram();

			std::optional<MaterialComponent::TextureInfo> optBaseColorMapInfo = pMaterialComponent->GetTextureInfo(cd::MaterialTextureType::BaseColor);
			if (optBaseColorMapInfo.has_value())
			{
				const MaterialComponent::TextureInfo& textureInfo = optBaseColorMapInfo.value();
				bgfx::setTexture(textureInfo.slot, bgfx::UniformHandle(textureInfo.samplerHandle), bgfx::TextureHandle(textureInfo.textureHandle));
			}

			std::optional<MaterialComponent::TextureInfo> optNormalMapInfo = pMaterialComponent->GetTextureInfo(cd::MaterialTextureType::Normal);
			if (optNormalMapInfo.has_value())
			{
				const MaterialComponent::TextureInfo& textureInfo = optNormalMapInfo.value();
				bgfx::setTexture(textureInfo.slot, bgfx::UniformHandle(textureInfo.samplerHandle), bgfx::TextureHandle(textureInfo.textureHandle));
			}

			std::optional<MaterialComponent::TextureInfo> optORMMapInfo = pMaterialComponent->GetTextureInfo(cd::MaterialTextureType::Metalness);
			if (optORMMapInfo.has_value())
			{
				const MaterialComponent::TextureInfo& textureInfo = optORMMapInfo.value();
				bgfx::setTexture(textureInfo.slot, bgfx::UniformHandle(textureInfo.samplerHandle), bgfx::TextureHandle(textureInfo.textureHandle));
			}

			constexpr StringCrc lutSampler("s_texLUT");
			constexpr StringCrc lutTexture("ibl_brdf_lut.dds");
			bgfx::setTexture(3, m_pRenderContext->GetUniform(lutSampler), m_pRenderContext->GetTexture(lutTexture));

			constexpr StringCrc cubeSampler("s_texCube");
			constexpr StringCrc cubeTexture("skybox/bolonga_lod.dds");
			bgfx::setTexture(4, m_pRenderContext->GetUniform(cubeSampler), m_pRenderContext->GetTexture(cubeTexture));
			
			constexpr StringCrc cubeIrrSampler("s_texCubeIrr");
			constexpr StringCrc cubeIrrTexture("skybox/bolonga_irr.dds");
			bgfx::setTexture(5, m_pRenderContext->GetUniform(cubeIrrSampler), m_pRenderContext->GetTexture(cubeIrrTexture));
		}
		else
		{
			// TODO...
		}

		uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;
		state |= BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);
		bgfx::submit(GetViewID(), shadingProgram);
	}
}

}