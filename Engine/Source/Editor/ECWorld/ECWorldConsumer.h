#pragma once

#include "Base/Template.h"
#include "ECWorld/Entity.h"
#include "Framework/IConsumer.h"

#include <memory>
#include <vector>

namespace cd
{

class SceneDatabase;

}

namespace engine
{

class MaterialType;
class RenderContext;
class World;

class ECWorldConsumer final : public cdtools::IConsumer
{
public:
	ECWorldConsumer() = delete;
	explicit ECWorldConsumer(World* pWorld, MaterialType* pMaterialType, RenderContext* pRenderContext) : m_pWorld(pWorld), m_pStandardMaterialType(pMaterialType), m_pRenderContext(pRenderContext) {}
	ECWorldConsumer(const ECWorldConsumer&) = delete;
	ECWorldConsumer& operator=(const ECWorldConsumer&) = delete;
	ECWorldConsumer(ECWorldConsumer&&) = delete;
	ECWorldConsumer& operator=(ECWorldConsumer&&) = delete;
	virtual ~ECWorldConsumer() = default;

	virtual void Execute(const cd::SceneDatabase* pSceneDatabase) override;

	std::vector<Entity>&& GetMeshEntities() { return cd::MoveTemp(m_meshEntities); }

private:
	RenderContext* m_pRenderContext;
	World* m_pWorld;
	MaterialType* m_pStandardMaterialType;

	std::vector<Entity> m_meshEntities;
};

}