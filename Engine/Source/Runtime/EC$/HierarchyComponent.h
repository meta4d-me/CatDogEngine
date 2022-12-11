#pragma once

namespace engine
{

class HierarchyComponent
{
public:
	HierarchyComponent() = default;
	HierarchyComponent(const HierarchyComponent&) = default;
	HierarchyComponent& operator=(const HierarchyComponent&) = default;
	HierarchyComponent(HierarchyComponent&&) = default;
	HierarchyComponent& operator=(HierarchyComponent&&) = default;
	~HierarchyComponent() = default;
};

}