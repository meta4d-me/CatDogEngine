#pragma once

#include "IResource.h"
#include "Scene/SceneDatabase.h"

#include <vector>

namespace cd
{

class Bone;

}

namespace engine
{

class SkeletonResource : public IResource
{
public:
	using VertexBuffer = std::vector<std::byte>;
	using IndexBuffer = std::vector<std::byte>;

public:
	SkeletonResource();
	SkeletonResource(const SkeletonResource&) = default;
	SkeletonResource& operator=(const SkeletonResource&) = default;
	SkeletonResource(SkeletonResource&&) = default;
	SkeletonResource& operator=(SkeletonResource&&) = default;
	virtual ~SkeletonResource();

	virtual void Update() override;
	virtual void Reset() override;
	
	void SetSceneDataBase(const cd::SceneDatabase* pSceneDataBase) {m_pSceneDatabase = pSceneDataBase; m_boneCount = m_pSceneDatabase->GetBoneCount();}

	uint32_t GetBoneCount() const { return m_boneCount; }
	uint16_t GetVertexBufferHandle() const { return m_vertexBufferHandle; }
	uint16_t GetIndexBufferHandle() const { return m_indexBufferHandle; }

private:
	void BuildSkeletonBuffer();
	void SubmitVertexBuffer();
	void SubmitIndexBuffer();
	void ClearSkeletonData();
	void FreeSkeletonData();
	void DestroyVertexBufferHandle();
	void DestroyIndexBufferHandle();

private:
	// Asset
	const cd::SceneDatabase* m_pSceneDatabase;
	uint32_t m_boneCount = 0U;

	// Runtime
	cd::VertexFormat m_currentVertexFormat;

	// CPU
	VertexBuffer m_vertexBuffer;
	IndexBuffer m_indexBuffer;
	uint32_t m_recycleCount = 0;

	// GPU
	uint16_t m_vertexBufferHandle = UINT16_MAX;
	uint16_t m_indexBufferHandle = UINT16_MAX;
};

}