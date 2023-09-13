#pragma once

#include "ECWorld/SceneWorld.h"
#include "Renderer.h"

namespace engine
{

	class PostProcessRenderer final : public Renderer
	{
	public:
		using Renderer::Renderer;
		virtual ~PostProcessRenderer();

		virtual void Init() override;
		virtual void Submit() override;
		virtual void UpdateView(const float* pViewMatrix, const float* pProjectionMatrix) override;
		virtual void Render(float deltaTime) override;

		virtual void SetEnable(bool value) override;
		virtual bool IsEnable() const override;

		void SetSceneWorld(SceneWorld* pSceneWorld) { m_pCurrentSceneWorld = pSceneWorld; }

	private:
		SceneWorld* m_pCurrentSceneWorld = nullptr;
	};

}