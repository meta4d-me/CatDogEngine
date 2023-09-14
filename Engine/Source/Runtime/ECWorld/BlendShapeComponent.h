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
	const std::vector<cd::Morph>* GetMorphs() { return  m_pMorphs;}
	void SetMesh(const cd::Mesh* mesh) { m_pMesh = mesh; }
	bool IsDirty() { return m_isDirty; }
	void SetIsDirty(bool isDirty) { m_isDirty = isDirty; }
	std::vector<float>& GetWeights() { return m_weights; };
	
	//void SetMorphAt() and record the changed one
	//std::vector<cd::Morph> GetMorphs() { return m_morphs; };
	float GetSourceMeshWeight() { return m_sourceMeshWeight; }
	
	uint16_t GetDynamicVertexBuffer() const { return m_dynamicVertexBufferHandle;}
	
	const float* GetDynamicVertexBufferData() const { return m_dynamicVertexBufferData.data(); }
	
	uint16_t GetStaticVertexBuffer() const { return m_staticVertexBufferHandle; }
	
	const float* GetStaticVertexBufferData() const { return m_staticVertexBufferData.data(); }

	void Reset();
	void Build();
	void Update();

private:
	//input
	const std::vector<cd::Morph>* m_pMorphs;
	std::vector<float> m_weights;
	const cd::Mesh* m_pMesh;
	bool m_isDirty;
	//output
	float m_sourceMeshWeight;
	std::vector<float> m_dynamicVertexBufferData;
	std::vector<float> m_staticVertexBufferData;
	std::vector<std::byte> m_dynamicVertexBuffer;
	std::vector<std::byte> m_staticVertexBuffer;
	uint16_t m_dynamicVertexBufferHandle = UINT16_MAX;
	uint16_t m_staticVertexBufferHandle = UINT16_MAX;
};

}