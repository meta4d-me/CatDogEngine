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
	const uint32_t GetActiveMorphCount() const { return m_activeMorphCount; }
	const uint32_t GetMorphCount() const { return static_cast<uint32_t>((*m_pMorphs).size()); }
	const std::vector<cd::Morph>* GetMorphs() { return  m_pMorphs;}
	void SetMesh(const cd::Mesh* mesh) { m_pMesh = mesh; }
	const uint32_t GetVertexCount() { return m_vertexCount; }
	
	bool IsDirty() { return m_isDirty; }
	void SetDirty(bool isDirty) { m_isDirty = isDirty; }
	std::vector<float>& GetWeights() { return m_weights; };
	
	//void SetMorphAt() and record the changed one
	//std::vector<cd::Morph> GetMorphs() { return m_morphs; };

	uint16_t GetVertexDynamicBuffer() const { return m_vertexDynamicBufferHandle;}
	uint16_t GetVertexStaticBuffer1() const { return m_vertexStaticBufferHandle1; }
	uint16_t GetVertexStaticBuffer2() const { return m_vertexStaticBufferHandle2; }
	uint16_t GetBlendShapeStaticBuffer1() const { return m_blendShapeStaticBufferHandle1; }
	uint16_t GetBlendShapeStaticBuffer2() const { return m_blendShapeStaticBufferHandle2; }
	uint16_t GetBlendShapeDynamifcBuffer1() const { return m_blendShapeDynamicBufferHandle1; }
	uint16_t GetBlendShapeDynamifcBuffer2() const { return m_blendShapeDynamicBufferHandle2; }
	uint16_t GetSourceWeightDynamicBuffer() const { return m_sourceWeightDynamicBufferHandle; }

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
	uint32_t m_vertexCount = 0U;
	uint32_t m_activeMorphCount = 0U;
	std::vector<std::byte>	m_vertexDynamicBuffer;								// Dynamic Vertex Buffer
	uint16_t						m_vertexDynamicBufferHandle = UINT16_MAX;
	std::vector<std::byte>	m_vertexStaticBuffer1;									// Vertex Buffer
	uint16_t						m_vertexStaticBufferHandle1 = UINT16_MAX;
	std::vector<std::byte>	m_vertexStaticBuffer2;										// Vertex Buffer
	uint16_t						m_vertexStaticBufferHandle2 = UINT16_MAX;
	std::vector<std::byte>	m_blendShapeStaticBuffer1;								// Index Buffer
	uint16_t						m_blendShapeStaticBufferHandle1 = UINT16_MAX;
	std::vector<std::byte>	m_blendShapeStaticBuffer2;								// Vertex Buffer
	uint16_t						m_blendShapeStaticBufferHandle2 = UINT16_MAX;
	std::vector<std::byte>	m_blendShapeDynamicBuffer1;						//	Dynamic Index Buffer
	uint16_t						m_blendShapeDynamicBufferHandle1 = UINT16_MAX;
	std::vector<std::byte>	m_blendShapeDynamicBuffer2;						//	Dynamic Vertex Buffer
	uint16_t						m_blendShapeDynamicBufferHandle2 = UINT16_MAX;
	std::vector<std::byte>	m_sourceWeightDynamicBuffer;						//	Dynamic vertex Buffer
	uint16_t						m_sourceWeightDynamicBufferHandle = UINT16_MAX;
};

}