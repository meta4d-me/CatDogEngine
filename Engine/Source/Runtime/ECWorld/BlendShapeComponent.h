#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/Entity.h"
#include "Scene/Mesh.h"

#include <cstdint>
#include <vector>

namespace cd
{

class Mesh;
class VertexFormat;

}

namespace engine
{

class World;

class BlendShapeComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("BlendShapeComponent");
		return className;
	}

public:
	BlendShapeComponent() = default;
	BlendShapeComponent(const BlendShapeComponent&) = default;
	BlendShapeComponent& operator=(const BlendShapeComponent&) = default;
	BlendShapeComponent(BlendShapeComponent&&) = default;
	BlendShapeComponent& operator=(BlendShapeComponent&&) = default;
	~BlendShapeComponent() = default;

	void SetMorphs(const std::vector<cd::Morph>& morphs) { m_pMorphs = &morphs;}
	void SetMesh(const cd::Mesh* mesh) { m_pMesh = mesh; }
	//void SetMorphAt() and record the changed one
	//std::vector<cd::Morph> GetMorphs() { return m_morphs; };
	float GetSourceMeshWeight() { return m_sourceMeshWeight; }

	void Reset();
	void Build();
	void Update();

private:
	//input
	const std::vector<cd::Morph>* m_pMorphs;
	const cd::Mesh* m_pMesh;
	
	//output
	float m_sourceMeshWeight;
	std::vector<float> m_dynamicVertexBufferData;
	std::vector<std::byte> m_dynamicVertexBuffer;
	uint16_t m_dynamicVertexBufferHandle = UINT16_MAX;
};

}