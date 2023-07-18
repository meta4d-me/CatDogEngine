#pragma once

#include "ComponentsStorage.hpp"
#include "Entity.h"
#include "Core/StringCrc.h"

#include <atomic>
#include <cassert>
#include <memory>
#include <vector>

namespace engine
{

// World is an area used to store and manage Entities, Components in the engine runtime.
// Usually, there is only one world shared between multiple threads.
class World
{
public:
	World() = default;
	World(const World&) = delete;
	World& operator=(const World&) = delete;
	World(World&&) = default;
	World& operator=(World&&) = default;
	~World() = default;

	Entity CreateEntity()
	{
		// Overflow is expected as I want to use the max value of EntityID as invalid value.
		// So I allocate entity id from 0.
		static std::atomic<Entity> nextEntity = INVALID_ENTITY + 1;
		return nextEntity.fetch_add(1);
	}

	template<typename Component>
	ComponentsStorage<Component>* Register()
	{
		StringCrc componentName = Component::GetClassName();
		assert(m_componentsLib.find(componentName.Value()) == m_componentsLib.end());
		m_componentsLib[componentName.Value()] = std::make_unique<ComponentsStorage<Component>>();
		return static_cast<ComponentsStorage<Component>*>(m_componentsLib[componentName.Value()].get());
	}

	template<typename Component>
	ComponentsStorage<Component>* GetComponents()
	{
		StringCrc componentName = Component::GetClassName();
		assert(m_componentsLib.contains(componentName.Value()));
		return static_cast<ComponentsStorage<Component>*>(m_componentsLib[componentName.Value()].get());
	}

	template<typename Component>
	Component& CreateComponent(Entity entity)
	{
		StringCrc componentName = Component::GetClassName();
		assert(m_componentsLib.find(componentName.Value()) != m_componentsLib.end());
		ComponentsStorage<Component>* pStorage = static_cast<ComponentsStorage<Component>*>(m_componentsLib[componentName.Value()].get());
		return pStorage->CreateComponent(entity);
	}

private:
	std::unordered_map<size_t, std::unique_ptr<IComponentsStorage>> m_componentsLib;
};

}