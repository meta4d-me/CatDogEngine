#pragma once

#include "Framework/IConsumer.h"
#include "MeshRenderData.h"

#include <string>

namespace cd
{

class SceneDatabase;

}

namespace engine
{

class BgfxConsumer final : public cdtools::IConsumer
{
public:
	BgfxConsumer() = delete;
	explicit BgfxConsumer(std::string filePath) : m_filePath(std::move(filePath)) {}
	BgfxConsumer(const BgfxConsumer&) = delete;
	BgfxConsumer& operator=(const BgfxConsumer&) = delete;
	BgfxConsumer(BgfxConsumer&&) = delete;
	BgfxConsumer& operator=(BgfxConsumer&&) = delete;
	virtual ~BgfxConsumer() = default;

	virtual void Execute(const cd::SceneDatabase* pSceneDatabase) override;
	RenderDataContext&& GetRenderDataContext() { return std::move(m_renderDataContext); }

private:
	std::string m_filePath;
	RenderDataContext m_renderDataContext;

	void ConvertMeshesFromScene(const cd::SceneDatabase& sceneDatabase, std::vector<MeshRenderData>& outLoadedMeshes) const;
	void GetMaterialsFromScene(const cd::SceneDatabase& sceneDatabase, std::vector<MaterialRenderData>& outLoadedMaterials) const;
};

}
