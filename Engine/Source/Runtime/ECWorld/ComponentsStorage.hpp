#pragma once

#include "Entity.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <unordered_map>
#include <vector>

namespace engine
{

class IComponentsStorage
{
public:
	virtual ~IComponentsStorage() = default;
};

// ComponentsStorage stores an array of Components in the same type and the entity which contains the component.
template<typename Component>
class ComponentsStorage : public IComponentsStorage
{
public:
	static_assert(!std::is_pointer_v<Component> && !std::is_reference_v<Component>);

public:
	ComponentsStorage() = default;
	ComponentsStorage(const ComponentsStorage&) = delete;
	ComponentsStorage& operator=(const ComponentsStorage&) = delete;
	ComponentsStorage(ComponentsStorage&&) = default;
	ComponentsStorage& operator=(ComponentsStorage&&) = default;
	virtual ~ComponentsStorage() = default;

	// Returns if ComponentStorage stores component for entity.
	bool Contains(Entity entity) const { return m_entityToIndex.find(entity) != m_entityToIndex.end(); }

	// Returns current active components count.
	size_t GetCount() const { return m_entityToIndex.size(); }

	// Returns current components capcity.
	size_t GetCapcity() const { assert(m_entities.size() == m_components.size()); return m_entities.size(); }

	// Need to check if it is still active.
	const std::vector<Entity>& GetEntities() const { return m_entities; }

	// Get component by entity.
	Component* GetComponent(Entity entity)
	{
		auto itIndex = m_entityToIndex.find(entity);
		return itIndex == m_entityToIndex.end() ? nullptr : &m_components[itIndex->second];
	}

	// Create component for entity.
	Component& CreateComponent(Entity entity)
	{
		assert(entity != INVALID_ENTITY && !Contains(entity));

		m_entityToIndex[entity] = m_components.size();
		m_entities.emplace_back(entity);
		m_components.emplace_back();
		return m_components.back();
	}

	// Remove actvie component from storage.
	void RemoveComponent(Entity entity)
	{
		auto itIndex = m_entityToIndex.find(entity);
		if (itIndex == m_entityToIndex.end())
		{
			return;
		}

		if (m_entityToIndex.size() > 1)
		{
			size_t unusedIndex = itIndex->second;
			m_entities[unusedIndex] = m_entities.back();
			m_components[unusedIndex] = cd::MoveTemp(m_components.back());
		}

		m_entities.pop_back();
		m_components.pop_back();
		m_entityToIndex.erase(entity);
	}

private:
	std::vector<Entity> m_entities;
	std::vector<Component> m_components;
	std::unordered_map<Entity, size_t> m_entityToIndex;
};

}