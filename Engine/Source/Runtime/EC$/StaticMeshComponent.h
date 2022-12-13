#pragma once

namespace engine
{

class StaticMeshComponent
{
public:
	StaticMeshComponent() = default;
	StaticMeshComponent(const StaticMeshComponent&) = default;
	StaticMeshComponent& operator=(const StaticMeshComponent&) = default;
	StaticMeshComponent(StaticMeshComponent&&) = default;
	StaticMeshComponent& operator=(StaticMeshComponent&&) = default;
	~StaticMeshComponent() = default;
};

}