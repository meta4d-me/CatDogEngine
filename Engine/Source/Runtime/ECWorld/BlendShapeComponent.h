#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/Entity.h"
#include "Scene/Mesh.h"

#include <cstdint>
#include <vector>
#include <map>

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

	void SetMesh(const cd::Mesh* mesh) { m_pMesh = mesh; m_meshVertexCount = m_pMesh->GetVertexCount();}
	const uint32_t GetMeshVertexCount() { return m_meshVertexCount; }

	void SetMorphs(const std::vector<cd::Morph>& morphs) { m_pMorphsData = morphs.data(); m_morphCount = static_cast<uint32_t>(morphs.size()); }
	const cd::Morph* GetMorphsData() { return  m_pMorphsData; }
	const uint32_t GetMorphCount() const { return m_morphCount; }
	const uint32_t GetActiveMorphCount() const { return m_activeMorphCount; }
	
	std::vector<float>& GetWeights() { return m_weights; };

	bool IsDirty() { return m_isDirty; }
	void SetDirty(bool isDirty) { m_isDirty = isDirty; }
	
	bool NeedUpdate() { return 0U != m_needUpdates.size(); }
	void AddNeedUpdate(uint32_t index, float weight) { m_needUpdates[index] = weight; }
	void ClearNeedUpdate() { m_needUpdates.clear(); }

	uint16_t GetFinalMorphAffectedVB() const { return m_finalMorphAffectedVBHandle; }
	uint16_t GetMorphAffectedVB() const { return m_morphAffectedVBHandle; }
	uint16_t GetNonMorphAffectedVB() const { return m_nonMorphAffectedVBHandle; }
	uint16_t GetAllMorphVertexIDIB() const { return m_allMorphVertexIDPosIBHandle; }
	uint16_t GetActiveMorphOffestLengthWeightIB() const { return m_activeMorphOffestLengthWeightIBHandle; }
	uint16_t GetChangedMorphIndexIB() const { return m_changedMorphIndexIBHandle; }

	void Reset();
	void Build();
	void Update();
	void UpdateChanged();

private:
	//input
	const cd::Mesh* m_pMesh;
	const cd::Morph* m_pMorphsData;
	std::vector<float> m_weights;
	
	uint32_t m_meshVertexCount = 0U;
	uint32_t m_morphCount = 0U;
	uint32_t m_activeMorphCount = 0U;
	uint32_t m_morphVertexCountSum = 0U;

	bool m_isDirty;
	std::map<uint32_t, float> m_needUpdates;
	//uint32_t m_needUpdate;
	
	//float m_updatedWeight = 0.0f;
	
	std::vector<std::byte>	m_morphAffectedVB;								
	uint16_t						m_morphAffectedVBHandle = UINT16_MAX;							// Vertex Buffer | Compute Input
	std::vector<std::byte>	m_nonMorphAffectedVB;
	uint16_t						m_nonMorphAffectedVBHandle = UINT16_MAX;					// Vertex Buffer | Vertex Input
	uint16_t						m_finalMorphAffectedVBHandle = UINT16_MAX;					// Dynamic Vertex Buffer | Compute Output | Vertex Input
	std::vector<std::byte>	m_allMorphVertexIDPosIB;												
	uint16_t						m_allMorphVertexIDPosIBHandle = UINT16_MAX;					// Index Buffer | Compute Input
	std::vector<std::byte>	m_activeMorphOffestLengthWeightIB;							
	uint16_t						m_activeMorphOffestLengthWeightIBHandle = UINT16_MAX;	//	Dynamic Index Buffer	| Compute Input
	std::vector<std::byte>	m_changedMorphIndexIB;
	uint16_t						m_changedMorphIndexIBHandle = UINT16_MAX;					//	Dynamic Index Buffer	| Compute Input
};

}