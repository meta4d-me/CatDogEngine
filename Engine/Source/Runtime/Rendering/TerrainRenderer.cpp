#include "TerrainRenderer.h"

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

	void TerrainRenderer::Init()
	{
		std::string sceneModelFilePath = "Models/generated_terrain.cdbin";
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

		// TODO use AssetPipeline to generate materials instead of hard-coding it
		m_materialHandles.reserve(2);
		int materialIndex = 0;
		uint64_t textureSamplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

		m_materialHandles.emplace_back();
		TerrainMaterialHandle& materialHandle = m_materialHandles.back();
		materialHandle.baseColor.sampler = m_pRenderContext->CreateUniform(std::format("s_textureBaseColor{}", materialIndex).c_str(), bgfx::UniformType::Sampler);
		materialHandle.baseColor.texture = m_pRenderContext->CreateTexture("Terrain_baseColor.dds", textureSamplerFlags | BGFX_TEXTURE_SRGB);
		materialHandle.debug.sampler = m_pRenderContext->CreateUniform(std::format("s_textureDebug{}", materialIndex).c_str(), bgfx::UniformType::Sampler);
		materialHandle.debug.texture = m_pRenderContext->CreateTexture("TerrainTest_baseColor.dds", textureSamplerFlags | BGFX_TEXTURE_SRGB);

		// Load terrain shaders
		bgfx::ShaderHandle vsh = m_pRenderContext->CreateShader("vs_terrain.bin");
		bgfx::ShaderHandle fsh = m_pRenderContext->CreateShader("fs_terrain.bin");
		m_program = m_pRenderContext->CreateProgram("TerrainProgram", vsh, fsh);

		// m_pRenderContext->GetCamera()->FrameAll(m_renderDataContext.sceneAABB);
	}

	void TerrainRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
	{
		bgfx::setViewFrameBuffer(GetViewID(), *m_pGBuffer->GetFrameBuffer());
		bgfx::setViewRect(GetViewID(), 0, 0, m_pGBuffer->GetWidth(), m_pGBuffer->GetHeight());
		bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
	}

	void TerrainRenderer::Render(float deltaTime)
	{
		for (size_t meshIndex = 0; meshIndex < m_meshHandles.size(); ++meshIndex)
		{
			const MeshHandle& meshHandle = m_meshHandles[meshIndex];
			const TerrainMaterialHandle& materialHandle = m_materialHandles[meshIndex];

			bgfx::setVertexBuffer(0, meshHandle.vbh);
			bgfx::setIndexBuffer(meshHandle.ibh);

			bgfx::setTexture(0, materialHandle.baseColor.sampler, materialHandle.baseColor.texture);
			bgfx::setTexture(1, materialHandle.debug.sampler, materialHandle.debug.texture);

			uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;
			state |= BGFX_STATE_DEPTH_TEST_LESS;
			bgfx::setState(state);

			bgfx::submit(GetViewID(), m_program);
		}
	}

}