#pragma once

#include "ECWorld/Entity.h"
#include "ECWorld/World.h"

#include <memory>
#include <vector>

namespace editor
{

class EditorWorld
{
public:
	EditorWorld();
	EditorWorld(const EditorWorld&) = default;
	EditorWorld& operator=(const EditorWorld&) = default;
	EditorWorld(EditorWorld&&) = default;
	EditorWorld& operator=(EditorWorld&&) = default;
	~EditorWorld() = default;

	engine::World* GetWorld() { return m_pWorld.get(); }
	const engine::World* GetWorld() const { return m_pWorld.get(); }
	engine::Entity GetSkyEntity() const { return m_skyEntity; }
	std::vector<engine::Entity>& GetMeshEntites() { return m_meshEntites; }
	const std::vector<engine::Entity>& GetMeshEntites() const { return m_meshEntites; }

private:
	std::unique_ptr<engine::World> m_pWorld;

	engine::Entity m_skyEntity;
	std::vector<engine::Entity> m_meshEntites;
};

}