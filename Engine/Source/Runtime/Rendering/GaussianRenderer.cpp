#include "GaussianRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ShaderResource.h"
#include "Log\Log.h"

namespace engine
{
void engine::GaussianRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("GaussianProgram", "vs_gaussianSplatting", "fs_gaussianSplatting"));

	bgfx::setViewName(GetViewID(), "GaussianRenderer");
	// NOTE: CreateTexture
	//const bgfx::Memory* mem = bgfx::copy(texturedata, sizeof(texturedata));
	//bgfx::TextureHandle texture = bgfx::createTexture(mem, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE);


	//NOTE : CreateTexture2D
	//uint16_t width = 1024;
	//uint16_t height = 1024; 
	//bool hasMips = false;  //if-mip-map
	//uint16_t numLayers = 1;
	//bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8; // fomat

	//const bgfx::Memory* mem = bgfx::copy(texturedata, sizeof(texturedata));
	//bgfx::TextureHandle texture = bgfx::createTexture2D(width, height, hasMips, numLayers, format, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, mem);
}

void engine::GaussianRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void engine::GaussianRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	CameraComponent* pMainCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
	const cd::Matrix4x4 camView = pMainCameraComponent->GetViewMatrix();
	const cd::Matrix4x4 camProj = pMainCameraComponent->GetProjectionMatrix();
	const cd::Matrix4x4 camViewProj = camProj * camView;

	for (Entity entity : m_pCurrentSceneWorld->GetGaussianRenderEntities())
	{
		if (auto* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			pTransformComponent->Build();
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
		}
		auto* pGaussianComponent = m_pCurrentSceneWorld->GetGaussianRenderComponent(entity);

		auto gaussianBuffer = pGaussianComponent->GetGaussianBuffer();
		auto gaussianVertexCount = pGaussianComponent->GetVertexCount();
		f_buffer.assign(reinterpret_cast<float*>(gaussianBuffer.data()),
			reinterpret_cast<float*>(gaussianBuffer.data() + gaussianBuffer.size()));

		float maxDepth = -std::numeric_limits<float>::infinity();
		float minDepth = std::numeric_limits<float>::infinity();

		sizeList.resize(gaussianVertexCount);
		for (size_t i = 0; i < gaussianVertexCount; ++i)
		{
			float depth = ((camViewProj.Data(0, 2) * f_buffer[8 * i + 0] +
				camViewProj.Data(1, 2) * f_buffer[8 * i + 1] +
				camViewProj.Data(2, 2) * f_buffer[8 * i + 2]) * 4096);
			sizeList[i] = static_cast<int32_t>(depth);
			if (depth > maxDepth) maxDepth = depth;
			if (depth < minDepth) minDepth = depth;
		}

		float depthInv = (256 * 256) / (maxDepth - minDepth);
		counts0.assign(256 * 256, 0);
		for (size_t i = 0; i < gaussianVertexCount; ++i)
		{
			sizeList[i] = static_cast<uint32_t>((sizeList[i] - minDepth) * depthInv);
			if (sizeList[i] == 65536) sizeList[i] -= 1;
			counts0[sizeList[i]]++;
		}

		starts0.assign(256 * 256, 0);
		for (int i = 1; i < 256 * 256; ++i)
		{
			starts0[i] = starts0[i - 1] + counts0[i - 1];
		}

		depthIndex.resize(gaussianVertexCount);
		for (uint32_t i = 0; i < gaussianVertexCount; ++i)
		{
			depthIndex[starts0[sizeList[i]]++] = i;
		}

		////Instace
		//const uint16_t instanceStride = 80;
		//// to total number of instances to draw
		//uint32_t totalSprites;
		//totalSprites = gaussianVertexCount;
		//uint32_t drawnSprites = bgfx::getAvailInstanceDataBuffer(totalSprites, instanceStride);

		//bgfx::InstanceDataBuffer idb;
		//bgfx::allocInstanceDataBuffer(&idb, drawnSprites, instanceStride);

		//uint8_t* data = idb.data;
		//for (uint32_t ii = 0; ii < drawnSprites; ++ii)
		//{
		//	float* mtx = (float*)data;
		///*	bx::mtxSRT(mtx, particleTransform.GetScale().x(), particleTransform.GetScale().y(), particleTransform.GetScale().z(),
		//		particleRotation.Pitch(), particleRotation.Yaw(), particleRotation.Roll(),
		//		pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().x(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().y(), pEmitterComponent->GetParticlePool().GetParticle(ii).GetPos().z());

		//	float* color = (float*)&data[64];
		//	color[0] = pEmitterComponent->GetEmitterColor().x();
		//	color[1] = pEmitterComponent->GetEmitterColor().y();
		//	color[2] = pEmitterComponent->GetEmitterColor().z();
		//	color[3] = pEmitterComponent->GetEmitterColor().w();*/

		//	data += instanceStride;
		//}

		//bgfx::setInstanceDataBuffer(&idb);
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pGaussianComponent->GetVertexBufferHandle() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pGaussianComponent->GetIndexBufferHandle() });


		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;
		bgfx::setState(state);

		constexpr StringCrc programHandleIndex{ "GaussianProgram" };
		GetRenderContext()->Submit(GetViewID(), programHandleIndex);
	}
}
}
