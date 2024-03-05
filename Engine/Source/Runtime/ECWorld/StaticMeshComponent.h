#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/Entity.h"
#include "Scene/Mesh.h"

#include <cstdint>
#include <vector>

namespace engine
{

class MeshResource;

class StaticMeshComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("StaticMeshComponent");
		return className;
	}

public:
	StaticMeshComponent() = default;
	StaticMeshComponent(const StaticMeshComponent&) = default;
	StaticMeshComponent& operator=(const StaticMeshComponent&) = default;
	StaticMeshComponent(StaticMeshComponent&&) = default;
	StaticMeshComponent& operator=(StaticMeshComponent&&) = default;
	~StaticMeshComponent() = default;

	const MeshResource* GetMeshResource() const { return m_pMeshResource; }
	void SetMeshResource(const MeshResource* pMeshResource);

	uint32_t GetStartVertex() const;
	uint32_t GetVertexCount() const;
	uint32_t GetStartIndex() const;
	uint32_t GetPolygonCount() const;
	uint32_t GetIndexCount() const;

private:
	const MeshResource* m_pMeshResource = nullptr;
	uint32_t m_currentVertexCount = UINT32_MAX;
	uint32_t m_currentPolygonCount = UINT32_MAX;
};

}