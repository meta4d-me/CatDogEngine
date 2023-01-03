#pragma once

#include "Entity.h"

#include <algorithm>
#include <cassert>
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
	bool Contains(Entity entity) const { return m_entityToIndex.contains(entity); }

	// Returns current active components count.
	size_t GetCount() const { return m_entityToIndex.size(); }

	// Returns current components capcity
	size_t GetCapcity() const { assert(m_entities.size() == m_components.size()); return m_entities.size(); }

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

		// There was an entity/component removed so we can reuse it.
		if (!m_unusedEntityIndexes.empty())
		{
			size_t reusedIndex = m_unusedEntityIndexes.back();
			m_unusedEntityIndexes.pop_back();

			m_entityToIndex[entity] = reusedIndex;
			m_entities[reusedIndex] = entity;
			return m_components[reusedIndex];
		}

		m_entityToIndex[entity] = m_components.size();
		m_entities.push_back(entity);
		m_components.push_back(Component());
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

		// We don't want to change the array immediately as it will cause memory copy/movement.
		// Instead, we mark it as it unused which will be removed in the future.
		m_unusedEntityIndexes.push_back(itIndex->second);
		m_entityToIndex.erase(entity);
	}

	// Remove unused components.
	void CleanUnused()
	{
		if (m_unusedEntityIndexes.empty())
		{
			// Components are all used.
			return;
		}

		if (m_entityToIndex.empty())
		{
			// Components are all unused.
			m_entities.clear();
			m_components.clear();
			m_entityToIndex.clear();
			m_unusedEntityIndexes.clear();
			return;
		}

		std::sort(m_unusedEntityIndexes.begin(), m_unusedEntityIndexes.end(),
			[](size_t lhs, size_t rhs) { return lhs < rhs; });

		// Back iterate to find last n active entities for unused entities to overwrite memory.
		int swapTimes = 0;
		int skipTimes = 0;
		std::unordered_map<Entity, size_t> tempEntityToIndex;
		for (int entityIndex = static_cast<int>(m_entities.size()) - 1; entityIndex >= 0; --entityIndex)
		{
			// Check if it is an active index.
			Entity entity = m_entities[entityIndex];
			if (m_entityToIndex.contains(entity))
			{
				size_t unusedIndex = m_unusedEntityIndexes[swapTimes++];
				m_entities[unusedIndex] = entity;
				m_components[unusedIndex] = std::move(m_components[entityIndex]);

				// Don't modify m_entityToIndex immediately in the process.
				// m_entityToIndex[entity] = unusedIndex;

				tempEntityToIndex[entity] = unusedIndex;
			}
			else
			{
				// No need to swap. Already at the array back.
				++skipTimes;
			}

			m_entities.pop_back();
			m_components.pop_back();

			// Finish.
			if (swapTimes + skipTimes == m_unusedEntityIndexes.size())
			{
				break;
			}
		}

		// Modify
		for (const auto& [entity, index] : tempEntityToIndex)
		{
			assert(m_entityToIndex.contains(entity));
			m_entityToIndex[entity] = index;
		}

		m_unusedEntityIndexes.clear();
	}

private:
	std::vector<Entity> m_entities;
	std::vector<Component> m_components;
	std::unordered_map<Entity, size_t> m_entityToIndex;
	std::vector<size_t> m_unusedEntityIndexes;
};

}