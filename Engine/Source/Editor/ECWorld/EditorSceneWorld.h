#pragma once

#include "ECWorld/Entity.h"
#include "ECWorld/World.h"
#include "Scene/SceneDatabase.h"

#include <memory>
#include <vector>

namespace editor
{

class EditorSceneWorld
{
public:
	EditorSceneWorld();
	EditorSceneWorld(const EditorSceneWorld&) = default;
	EditorSceneWorld& operator=(const EditorSceneWorld&) = default;
	EditorSceneWorld(EditorSceneWorld&&) = default;
	EditorSceneWorld& operator=(EditorSceneWorld&&) = default;
	~EditorSceneWorld() = default;

	cd::SceneDatabase* GetSceneDatabase() { return m_pSceneDatabase.get(); }
	engine::World* GetWorld() { return m_pWorld.get(); }
	const engine::World* GetWorld() const { return m_pWorld.get(); }
	const engine::Entity GetSkyEntity() const { return m_skyEntity; }
	std::vector<engine::Entity>& GetMeshEntites() { return m_meshEntites; }
	const std::vector<engine::Entity>& GetMeshEntites() const { return m_meshEntites; }

private:
	std::unique_ptr<cd::SceneDatabase> m_pSceneDatabase;
	std::unique_ptr<engine::World> m_pWorld;

	engine::Entity m_skyEntity;
	std::vector<engine::Entity> m_meshEntites;
};

}