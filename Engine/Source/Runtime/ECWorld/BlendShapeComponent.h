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

	void SetMorphs(const std::vector<cd::Morph>& morphs) { m_pMorphs = &morphs; }
	const uint32_t GetActiveMorphCount() const { return m_activeMorphCount; }
	const uint32_t GetMorphCount() const { return static_cast<uint32_t>((*m_pMorphs).size()); }
	const std::vector<cd::Morph>* GetMorphs() { return  m_pMorphs; }
	void SetMesh(const cd::Mesh* mesh) { m_pMesh = mesh; }
	const uint32_t GetVertexCount() { return m_vertexCount; }

	bool IsDirty() { return m_isDirty; }
	bool Is2Update() { return !(m_is2Update == static_cast<uint32_t>((*m_pMorphs).size())); }
	void SetDirty(bool isDirty) { m_isDirty = isDirty; }
	void Set2UpdateFalse() { m_is2Update = static_cast<uint32_t>((*m_pMorphs).size()); }
	void Set2Update(uint32_t index, float weight) { m_is2Update = index; m_updatedWeight = weight; }
	std::vector<float>& GetWeights() { return m_weights; };

	uint32_t Get2Update() { return m_is2Update; }
	float GetUpdatedWeight() { return m_updatedWeight; }

	//void SetMorphAt() and record the changed one
	//std::vector<cd::Morph> GetMorphs() { return m_morphs; };

	uint16_t GetFinalMorphAffectedVB() const { return m_finalMorphAffectedVBHandle; }
	uint16_t GetMorphAffectedVB() const { return m_morphAffectedVBHandle; }
	uint16_t GetNonMorphAffectedVB() const { return m_nonMorphAffectedVBHandle; }
	uint16_t GetAllMorphVertexIDIB() const { return m_allMorphVertexIDIBHandle; }
	uint16_t GetAllMorphVertexPosVB() const { return m_allMorphVertexPosVBHandle; }

	uint16_t GetActiveMorphOffestLengthIB() const { return m_activeMorphOffestLengthIBHandle; }
	uint16_t GetActiveMorphWeightVB() const { return m_activeMorphWeightVBHandle; }
	uint16_t GetChangedMorphIndexIB() const { return m_changedMorphIndexIBHandle; }

	void Reset();
	void Build();
	void Update();
	void UpdateChanged();

private:
	//input
	const std::vector<cd::Morph>* m_pMorphs;
	std::vector<float> m_weights;
	const cd::Mesh* m_pMesh;
	bool m_isDirty;
	uint32_t m_is2Update;
	float m_updatedWeight = 0.0f;
	
	uint32_t m_vertexCount = 0U;
	uint32_t m_activeMorphCount = 0U;
	uint32_t m_morphVertexCountSum = 0U;

	std::vector<std::byte>	m_morphAffectedVB;								
	uint16_t						m_morphAffectedVBHandle = UINT16_MAX;					// Vertex Buffer | Compute Input
	std::vector<std::byte>	m_nonMorphAffectedVB;
	uint16_t						m_nonMorphAffectedVBHandle = UINT16_MAX;			// Vertex Buffer | Vertex Input
	uint16_t						m_finalMorphAffectedVBHandle = UINT16_MAX;			// Dynamic Vertex Buffer | Compute Output | Vertex Input

	std::vector<std::byte>	m_allMorphVertexIDIB;												// Index Buffer | Compute Input
	uint16_t						m_allMorphVertexIDIBHandle = UINT16_MAX;
	std::vector<std::byte>	m_allMorphVertexPosVB;											// Vertex Buffer | Compute Input
	uint16_t						m_allMorphVertexPosVBHandle = UINT16_MAX;
	std::vector<std::byte>	m_activeMorphOffestLengthIB;							
	uint16_t						m_activeMorphOffestLengthIBHandle = UINT16_MAX;	//	Dynamic Index Buffer	| Compute Input
	std::vector<std::byte>	m_activeMorphWeightVB;									
	uint16_t						m_activeMorphWeightVBHandle = UINT16_MAX;			//	Dynamic Vertex Buffer | Compute Input

	uint16_t						m_changedMorphIndexIBHandle = UINT16_MAX;	//	Dynamic Index Buffer	| Compute Input
};

}