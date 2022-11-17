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
	cdtools::BgfxConsumer bgfxConsumer("");
	cdtools::Processor processor(&cdProducer, &bgfxConsumer);
	processor.Run();

	m_vertexLayout
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.end();

	// Start creating bgfx resources from RenderDataContext
	m_renderDataContext = bgfxConsumer.GetRenderDataContext();

	m_meshHandles.reserve(m_renderDataContext.meshRenderDataArray.size());
	for (const MeshRenderData& meshRenderData : m_renderDataContext.meshRenderDataArray)
	{
		MeshHandle meshHandle;
		const bgfx::Memory* pVBMemory = bgfx::makeRef(meshRenderData.vertices.data(), meshRenderData.GetVerticesBufferLength());
		meshHandle.vbh = bgfx::createVertexBuffer(pVBMemory, m_vertexLayout);
		
		const bgfx::Memory* pIBMemory = bgfx::makeRef(meshRenderData.indices.data(), meshRenderData.GetIndicesBufferLength());
		meshHandle.ibh = bgfx::createIndexBuffer(pIBMemory, BGFX_BUFFER_INDEX32);

		m_meshHandles.emplace_back(std::move(meshHandle));
	}

	m_materialHandles.reserve(m_renderDataContext.materialRenderDataArray.size());
	int materialIndex = 0;
	uint64_t textureSamplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	for (const MaterialRenderData& materialRenderData : m_renderDataContext.materialRenderDataArray)
	{
		PBRMaterialHandle materialHandle;

		const std::optional<std::string>& optBaseColor = materialRenderData.GetTextureName(cdtools::MaterialTextureType::BaseColor);
		if (optBaseColor.has_value())
		{
			TextureHandle textureHandle;
			textureHandle.sampler = m_pRenderContext->CreateUniform(std::format("s_textureBaseColor{}", materialIndex).c_str(), bgfx::UniformType::Sampler);
			textureHandle.texture = m_pRenderContext->CreateTexture((optBaseColor.value() + ".dds").c_str(), textureSamplerFlags | BGFX_TEXTURE_SRGB);
			materialHandle.baseColor = std::move(textureHandle);
		}
		
		const std::optional<std::string>& optNormal = materialRenderData.GetTextureName(cdtools::MaterialTextureType::Normal);
		if (optNormal.has_value())
		{
			TextureHandle textureHandle;
			textureHandle.sampler = m_pRenderContext->CreateUniform(std::format("s_textureNormal{}", materialIndex).c_str(), bgfx::UniformType::Sampler);
			textureHandle.texture = m_pRenderContext->CreateTexture((optNormal.value() + ".dds").c_str(), textureSamplerFlags);
			materialHandle.normal = std::move(textureHandle);
		}

		const std::optional<std::string>& optRoughness = materialRenderData.GetTextureName(cdtools::MaterialTextureType::Roughness);
		if (optRoughness.has_value())
		{
			TextureHandle textureHandle;
			textureHandle.sampler = m_pRenderContext->CreateUniform(std::format("s_textureORM{}", materialIndex).c_str(), bgfx::UniformType::Sampler);
			textureHandle.texture = m_pRenderContext->CreateTexture((optRoughness.value() + ".dds").c_str(), textureSamplerFlags);
			materialHandle.orm = std::move(textureHandle);
		}

		m_materialHandles.emplace_back(std::move(materialHandle));
		++materialIndex;
	}

	bgfx::ShaderHandle vsh = m_pRenderContext->CreateShader("vs_PBR.bin");
	bgfx::ShaderHandle fsh = m_pRenderContext->CreateShader("fs_PBR_0.bin");
	m_programPBR = m_pRenderContext->CreateProgram("PBR", vsh, fsh);

	// Let camera focus on the loaded scene by default.
	m_pRenderContext->GetCamera()->FrameAll(m_renderDataContext.sceneAABB);
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

		bgfx::setTexture(0, m_pRenderContext->GetUniform(StringCrc("s_texCube")),
			m_pRenderContext->GetTexture(StringCrc("skybox/bolonga_lod.dds")));
		bgfx::setTexture(1, m_pRenderContext->GetUniform(StringCrc("s_texCubeIrr")),
			m_pRenderContext->GetTexture(StringCrc("skybox/bolonga_irr.dds")));
		bgfx::setTexture(5, m_pRenderContext->GetUniform(StringCrc("s_texLUT")),
			m_pRenderContext->GetTexture(StringCrc("ibl_brdf_lut.dds")));

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