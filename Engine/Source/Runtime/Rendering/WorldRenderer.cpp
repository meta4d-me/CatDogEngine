#include "WorldRenderer.h"

#include "ECWorld/MaterialComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ECWorld/World.h"
#include "Display/Camera.h"
#include "RenderContext.h"
#include "Scene/Texture.h"

#include <bgfx/bgfx.h>
#include <bx/math.h>

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

	for (Entity entity : *m_pMeshEntites)
	{
		StaticMeshComponent* pMeshComponent = pMeshStorage->GetComponent(entity);
		if (!pMeshComponent)
		{
			continue;
		}

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle(pMeshComponent->GetVertexBuffer()));
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle(pMeshComponent->GetIndexBuffer()));

		bgfx::ProgramHandle shadingProgram(BGFX_INVALID_HANDLE);
		MaterialComponent* pMaterialComponent = pMaterialStorage->GetComponent(entity);
		if (pMaterialComponent)
		{
			shadingProgram.idx = pMaterialComponent->GetShadingProgram();

			const auto& baseColorMapInfo = pMaterialComponent->GetTextureInfo(cd::MaterialTextureType::BaseColor);
			bgfx::setTexture(baseColorMapInfo.slot, bgfx::UniformHandle(baseColorMapInfo.samplerHandle), bgfx::TextureHandle(baseColorMapInfo.textureHandle));

			const auto& normalMapInfo = pMaterialComponent->GetTextureInfo(cd::MaterialTextureType::Normal);
			bgfx::setTexture(normalMapInfo.slot, bgfx::UniformHandle(normalMapInfo.samplerHandle), bgfx::TextureHandle(normalMapInfo.textureHandle));

			const auto& ormMapInfo = pMaterialComponent->GetTextureInfo(cd::MaterialTextureType::Metalness);
			bgfx::setTexture(ormMapInfo.slot, bgfx::UniformHandle(ormMapInfo.samplerHandle), bgfx::TextureHandle(ormMapInfo.textureHandle));

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