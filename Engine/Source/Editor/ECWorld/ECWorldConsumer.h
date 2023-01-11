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

}

namespace editor
{

class ECWorldConsumer final : public cdtools::IConsumer
{
public:
	ECWorldConsumer() = delete;
	explicit ECWorldConsumer(engine::World* pWorld, engine::MaterialType* pMaterialType, engine::RenderContext* pRenderContext) : m_pWorld(pWorld), m_pStandardMaterialType(pMaterialType), m_pRenderContext(pRenderContext) {}
	ECWorldConsumer(const ECWorldConsumer&) = delete;
	ECWorldConsumer& operator=(const ECWorldConsumer&) = delete;
	ECWorldConsumer(ECWorldConsumer&&) = delete;
	ECWorldConsumer& operator=(ECWorldConsumer&&) = delete;
	virtual ~ECWorldConsumer() = default;

	void SetSceneDatabaseIDs(uint32_t meshID);
	virtual void Execute(const cd::SceneDatabase* pSceneDatabase) override;

	std::vector<engine::Entity>&& GetMeshEntities() { return cd::MoveTemp(m_meshEntities); }

private:
	engine::RenderContext* m_pRenderContext;
	engine::World* m_pWorld;
	engine::MaterialType* m_pStandardMaterialType;

	std::vector<engine::Entity> m_meshEntities;
	uint32_t m_meshMinID;
};

}