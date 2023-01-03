#pragma once

#include "Core/StringCrc.h"

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
};

}