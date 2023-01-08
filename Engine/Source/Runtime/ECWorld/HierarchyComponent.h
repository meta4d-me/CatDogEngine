#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/HierarchyComponent.h"

namespace engine
{

class HierarchyComponent
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("HierarchyComponent");
		return className;
	}

public:
	HierarchyComponent() = default;
	HierarchyComponent(const HierarchyComponent&) = default;
	HierarchyComponent& operator=(const HierarchyComponent&) = default;
	HierarchyComponent(HierarchyComponent&&) = default;
	HierarchyComponent& operator=(HierarchyComponent&&) = default;
	~HierarchyComponent() = default;

	void SetParentEntity(Entity entity) { m_parentEntity = entity; }
	Entity GetParentEntity() const { return m_parentEntity; }

private:
	Entity m_parentEntity = INVALID_ENTITY;
};

}