#include "SceneRenderer.h"

#include "BgfxConsumer.h"
#include "Display/Camera.h"
#include "GBuffer.h"
#include "Producer/CatDogProducer.h"
#include "Processor/Processor.h"
#include "RenderContext.h"
#include "Scene/Texture.h"
#include "SwapChain.h"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <format>

namespace engine
{

void SceneRenderer::Init()
{
	// CatDogProducer can parse .catdog.bin format file to SceneDatabase in memory.
	// BgfxConsumer is used to translate SceneDatabase to data which bgfx api can use directly.
	std::string sceneModelFilePath = "Models/kitchen_tools.cdbin";
	cdtools::CatDogProducer cdProducer(CDENGINE_RESOURCES_ROOT_PATH + sceneModelFilePath);
	BgfxConsumer bgfxConsumer("");
	cdtools::Processor processor(&cdProducer, &bgfxConsumer);
	processor.Run();

	// Start creating bgfx resources from RenderDataContext
	m_renderDataContext = bgfxConsumer.GetRenderDataContext();

	m_meshHandles.reserve(m_renderDataContext.meshRenderDataArray.size());
	for (const MeshRenderData& meshRenderData : m_renderDataContext.meshRenderDataArray)
	{
		m_meshHandles.emplace_back();
		MeshHandle& meshHandle = m_meshHandles.back();

		const bgfx::Memory* pVBMemory = bgfx::makeRef(static_cast<const void*>(meshRenderData.GetRawVertices().data()), meshRenderData.GetVerticesBufferLength());
		meshHandle.vbh = bgfx::createVertexBuffer(pVBMemory, meshRenderData.GetVertexLayout());
		
		const bgfx::Memory* pIBMemory = bgfx::makeRef(static_cast<const void*>(meshRenderData.GetIndices().data()), meshRenderData.GetIndicesBufferLength());
		meshHandle.ibh = bgfx::createIndexBuffer(pIBMemory, BGFX_BUFFER_INDEX32);
	}

	m_materialHandles.reserve(m_renderDataContext.materialRenderDataArray.size());
	int materialIndex = 0;
	uint64_t textureSamplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	for (const MaterialRenderData& materialRenderData : m_renderDataContext.materialRenderDataArray)
	{
		m_materialHandles.emplace_back();
		PBRMaterialHandle& materialHandle = m_materialHandles.back();

		const std::optional<std::string>& optBaseColor = materialRenderData.GetTextureName(cd::MaterialTextureType::BaseColor);
		if (optBaseColor.has_value())
		{
			materialHandle.baseColor.sampler = m_pRenderContext->CreateUniform(std::format("s_textureBaseColor{}", materialIndex).c_str(), bgfx::UniformType::Sampler);
			materialHandle.baseColor.texture = m_pRenderContext->CreateTexture((optBaseColor.value() + ".dds").c_str(), textureSamplerFlags | BGFX_TEXTURE_SRGB);
		}
		
		const std::optional<std::string>& optNormal = materialRenderData.GetTextureName(cd::MaterialTextureType::Normal);
		if (optNormal.has_value())
		{
			materialHandle.normal.sampler = m_pRenderContext->CreateUniform(std::format("s_textureNormal{}", materialIndex).c_str(), bgfx::UniformType::Sampler);
			materialHandle.normal.texture = m_pRenderContext->CreateTexture((optNormal.value() + ".dds").c_str(), textureSamplerFlags);
		}

		const std::optional<std::string>& optRoughness = materialRenderData.GetTextureName(cd::MaterialTextureType::Roughness);
		if (optRoughness.has_value())
		{
			materialHandle.orm.sampler = m_pRenderContext->CreateUniform(std::format("s_textureORM{}", materialIndex).c_str(), bgfx::UniformType::Sampler);
			materialHandle.orm.texture = m_pRenderContext->CreateTexture((optRoughness.value() + ".dds").c_str(), textureSamplerFlags);
		}

		++materialIndex;
	}

	bgfx::ShaderHandle vsh = m_pRenderContext->CreateShader("vs_PBR.bin");
	bgfx::ShaderHandle fsh = m_pRenderContext->CreateShader("fs_PBR_0.bin");
	m_programPBR = m_pRenderContext->CreateProgram("PBR", vsh, fsh);

	// Let camera focus on the loaded scene by default.
	m_pRenderContext->GetCamera()->FrameAll(m_renderDataContext.sceneAABB);

	m_pRenderContext->CreateUniform("s_texCube", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform("s_texCubeIrr", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform("s_texLUT", bgfx::UniformType::Sampler);
	uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
	m_pRenderContext->CreateTexture("skybox/bolonga_lod.dds", samplerFlags);
	m_pRenderContext->CreateTexture("skybox/bolonga_irr.dds", samplerFlags);
	m_pRenderContext->CreateTexture("ibl_brdf_lut.dds");
}

void SceneRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bgfx::setViewFrameBuffer(GetViewID(), *m_pGBuffer->GetFrameBuffer());
	bgfx::setViewRect(GetViewID(), 0, 0, m_pGBuffer->GetWidth(), m_pGBuffer->GetHeight());
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void SceneRenderer::Render(float deltaTime)
{
	for (size_t meshIndex = 0; meshIndex < m_meshHandles.size(); ++meshIndex)
	{
		const MeshHandle& meshHandle = m_meshHandles[meshIndex];
		const PBRMaterialHandle& materialHandle = m_materialHandles[meshIndex];
	
		bgfx::setVertexBuffer(0, meshHandle.vbh);
		bgfx::setIndexBuffer(meshHandle.ibh);

		constexpr StringCrc cubeSampler("s_texCube");
		constexpr StringCrc cubeTexture("skybox/bolonga_lod.dds");
		bgfx::setTexture(0, m_pRenderContext->GetUniform(cubeSampler), m_pRenderContext->GetTexture(cubeTexture));

		constexpr StringCrc cubeIrrSampler("s_texCubeIrr");
		constexpr StringCrc cubeIrrTexture("skybox/bolonga_irr.dds");
		bgfx::setTexture(1, m_pRenderContext->GetUniform(cubeIrrSampler), m_pRenderContext->GetTexture(cubeIrrTexture));

		constexpr StringCrc lutSampler("s_texLUT");
		constexpr StringCrc lutTexture("ibl_brdf_lut.dds");
		bgfx::setTexture(5, m_pRenderContext->GetUniform(lutSampler), m_pRenderContext->GetTexture(lutTexture));

		bgfx::setTexture(2, materialHandle.baseColor.sampler, materialHandle.baseColor.texture);
		bgfx::setTexture(3, materialHandle.normal.sampler, materialHandle.normal.texture);
		bgfx::setTexture(4, materialHandle.orm.sampler, materialHandle.orm.texture);
	
		uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;
		state |= BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);
	
		bgfx::submit(GetViewID(), m_programPBR);
	}
}

}