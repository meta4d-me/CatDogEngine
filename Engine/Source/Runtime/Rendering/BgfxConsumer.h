#pragma once

#include "Consumer/IConsumer.h"
#include "Scene/SceneDatabase.h"
#include "MeshRenderData.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <assert.h>

namespace cdtools
{

	class BgfxConsumer final : public IConsumer
	{
	public:
		BgfxConsumer() = delete;
		BgfxConsumer(std::string filePath);

		virtual void Execute(const SceneDatabase *pSceneDatabase) override;
		RenderDataContext &&GetRenderDataContext();

	private:
		std::string m_filePath;
		RenderDataContext m_renderDataContext;
	};

}
