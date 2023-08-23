#pragma once

#include "ECWorld/SceneWorld.h"
#include "Renderer.h"
#include<bgfx/bgfx.h>
#include<vector>

constexpr int TEX_CHAIN_LEN = 9;

namespace engine
{

	class BloomRenderer final : public Renderer
	{
	public:
		using Renderer::Renderer;
		virtual ~BloomRenderer();

		virtual void Init() override;
		virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
		virtual void Render(float deltaTime) override;

		virtual void SetEnable(bool value) override;
		virtual bool IsEnable() const override;

		void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

	private:
		void Blur(uint16_t width , uint16_t height,int iteration, float blursize, int blurscaling,cd::Matrix4x4 ortho, bgfx::TextureHandle texture);

		SceneWorld* m_pCurrentSceneWorld = nullptr;

		bgfx::FrameBufferHandle m_blurChainFB[2];
		bgfx::FrameBufferHandle m_sampleChainFB[TEX_CHAIN_LEN];
		bgfx::FrameBufferHandle m_combineFB;

		uint16_t width;
		uint16_t height;

		uint16_t start_dowmSamplePassID;
		uint16_t start_verticalBlurPassID;
		uint16_t start_horizontalBlurPassID;
		uint16_t start_upSamplePassID;
		uint16_t blit_colorPassID;
		uint16_t combinePassID;
	};

}