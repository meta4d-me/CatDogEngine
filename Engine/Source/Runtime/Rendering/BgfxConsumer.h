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
	explicit BgfxConsumer(std::string filePath) : m_filePath(std::move(filePath)) {}
	BgfxConsumer(const BgfxConsumer&) = delete;
	BgfxConsumer& operator=(const BgfxConsumer&) = delete;
	BgfxConsumer(BgfxConsumer&&) = delete;
	BgfxConsumer& operator=(BgfxConsumer&&) = delete;
	virtual ~BgfxConsumer() = default;

	virtual void Execute(const SceneDatabase* pSceneDatabase) override;
	RenderDataContext&& GetRenderDataContext() { return std::move(m_renderDataContext); }

private:
	std::string m_filePath;
	RenderDataContext m_renderDataContext;
};

}
