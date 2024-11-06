#include "SkeletonResource.h"

#include "Log/Log.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Utilities/MeshUtils.hpp"

namespace details
{

void TraverseBone(const cd::Bone& bone, const cd::SceneDatabase* pSceneDatabase, std::byte* currentDataPtr,
	std::byte* currentIndexPtr, uint32_t& vertexOffset, uint32_t& indexOffset)
{
	constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
	constexpr uint32_t indexDataSize = sizeof(uint16_t);
	for (auto& child : bone.GetChildIDs())
	{
		const cd::Bone& currBone = pSceneDatabase->GetBone(child.Data());
		cd::Vec3f translate = currBone.GetOffset().Inverse().GetTranslation();

		uint16_t parentID = currBone.GetParentID().Data();
		uint16_t currBoneID = currBone.GetID().Data();
		uint16_t selectedBoneIndex[4] = { currBoneID, currBoneID, currBoneID, currBoneID };
		std::vector<uint16_t> vertexBoneIndexes;
		vertexBoneIndexes.resize(4, 0U);
		vertexBoneIndexes[0] = currBoneID;
		std::memcpy(&currentDataPtr[vertexOffset], translate.begin(), posDataSize);
		vertexOffset += posDataSize;
		std::memcpy(&currentDataPtr[vertexOffset], selectedBoneIndex, static_cast<uint32_t>(4 * sizeof(uint16_t)));
		vertexOffset += static_cast<uint32_t>(4 * sizeof(uint16_t));
		std::memcpy(&currentIndexPtr[indexOffset], &parentID, indexDataSize);
		indexOffset += indexDataSize;
		std::memcpy(&currentIndexPtr[indexOffset], &currBoneID, indexDataSize);
		indexOffset += indexDataSize;

		TraverseBone(currBone, pSceneDatabase, currentDataPtr, currentIndexPtr, vertexOffset, indexOffset);
	}
}

}

namespace engine
{

SkeletonResource::SkeletonResource() = default;

SkeletonResource::~SkeletonResource()
{
	// Collect garbage intermediatly.
	//SetStatus(ResourceStatus::Garbage);
	//Update();
}


void SkeletonResource::Update()
{
	switch (GetStatus())
	{
	case ResourceStatus::Loading:
	{
		if (m_pSceneDatabase)
		{
			m_boneCount = m_pSceneDatabase->GetBoneCount();
			SetStatus(ResourceStatus::Loaded);
		}
		break;
	}
	case ResourceStatus::Loaded:
	{
		if (m_boneCount > 0U)
		{
			SetStatus(ResourceStatus::Building);
		}
		SetStatus(ResourceStatus::Building);
		break;
	}
	case ResourceStatus::Building:
	{
		BuildSkeletonBuffer();
		SetStatus(ResourceStatus::Built);
		break;
	}
	case ResourceStatus::Built:
	{
		SubmitVertexBuffer();
		SubmitIndexBuffer();
		m_recycleCount = 0U;
		SetStatus(ResourceStatus::Ready);
		break;
	}
	case ResourceStatus::Ready:
	{
		// Release CPU data later to save memory.
		constexpr uint32_t recycleDelayFrames = 30U;
		if (m_recycleCount++ >= recycleDelayFrames)
		{
			FreeSkeletonData();
			SetStatus(ResourceStatus::Optimized);
		}
		break;
	}
	case ResourceStatus::Garbage:
	{
		DestroyVertexBufferHandle();
		DestroyIndexBufferHandle();
		// CPU data will destroy after deconstructor.
		SetStatus(ResourceStatus::Destroyed);
		break;
	}
	default:
		break;
	}
}

void SkeletonResource::Reset()
{
	DestroyVertexBufferHandle();
	DestroyIndexBufferHandle();
	ClearSkeletonData();
	SetStatus(ResourceStatus::Loading);
}

void SkeletonResource::BuildSkeletonBuffer()
{
	constexpr uint32_t indexTypeSize = static_cast<uint32_t>(sizeof(uint16_t));
	constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
	m_currentVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);
	m_currentVertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::BoneIndex, cd::AttributeValueType::Int16, 4U);
	m_indexBuffer.resize((m_boneCount - 1) * 2 * indexTypeSize);
	m_vertexBuffer.resize(m_boneCount * m_currentVertexFormat.GetStride());

	uint32_t vbDataSize = 0U;
	uint32_t ibDataSize = 0U;

	auto vbDataPtr = m_vertexBuffer.data();
	const cd::Point& position = m_pSceneDatabase->GetBone(0).GetTransform().GetTranslation();
	std::memcpy(&vbDataPtr[vbDataSize], position.begin(), posDataSize);
	vbDataSize += posDataSize;
	//std::vector<uint16_t> vertexBoneIndexes;
	uint16_t selectedBoneIndex[4] = { 0U, 0U, 0U, 0U };
	//vertexBoneIndexes.resize(4, 0U);
	//vertexBoneIndexes[0] = 0U;
	//uint16_t first = 0;
	std::memcpy(&vbDataPtr[vbDataSize], selectedBoneIndex, static_cast<uint32_t>(4 * sizeof(uint16_t)));
	vbDataSize += static_cast<uint32_t>(4 * sizeof(uint16_t));

	details::TraverseBone(m_pSceneDatabase->GetBone(0), m_pSceneDatabase, m_vertexBuffer.data(), m_indexBuffer.data(), vbDataSize, ibDataSize);
}

void SkeletonResource::SubmitVertexBuffer()
{
	if (m_vertexBufferHandle != UINT16_MAX)
	{
		return;
	}

	bgfx::VertexLayout vertexLayout;
	engine::VertexLayoutUtility::CreateVertexLayout(vertexLayout, m_currentVertexFormat.GetVertexAttributeLayouts());
	const bgfx::Memory* pVertexBufferRef = bgfx::makeRef(m_vertexBuffer.data(), static_cast<uint32_t>(m_vertexBuffer.size()));
	bgfx::VertexBufferHandle vertexBufferHandle = bgfx::createVertexBuffer(pVertexBufferRef, vertexLayout);
	assert(bgfx::isValid(vertexBufferHandle));
	m_vertexBufferHandle = vertexBufferHandle.idx;
}

void SkeletonResource::SubmitIndexBuffer()
{
	if (m_indexBufferHandle != UINT16_MAX)
	{
		return;
	}

	const auto& indexBuffer = m_indexBuffer;
	assert(!m_indexBuffer.empty());
	const bgfx::Memory* pIndexBufferRef = bgfx::makeRef(indexBuffer.data(), static_cast<uint32_t>(indexBuffer.size()));

	bgfx::IndexBufferHandle indexBufferHandle = bgfx::createIndexBuffer(pIndexBufferRef);
	assert(bgfx::isValid(indexBufferHandle));
	m_indexBufferHandle = indexBufferHandle.idx;
}

void SkeletonResource::ClearSkeletonData()
{
	m_vertexBuffer.clear();
	m_indexBuffer.clear();
}

void SkeletonResource::FreeSkeletonData()
{
	ClearSkeletonData();
	VertexBuffer().swap(m_vertexBuffer);
	IndexBuffer().swap(m_indexBuffer);
}

void SkeletonResource::DestroyVertexBufferHandle()
{
	if (m_vertexBufferHandle != UINT16_MAX)
	{
		bgfx::destroy(bgfx::VertexBufferHandle{ m_vertexBufferHandle });
		m_vertexBufferHandle = UINT16_MAX;
	}
}

void SkeletonResource::DestroyIndexBufferHandle()
{
	if (m_indexBufferHandle != UINT16_MAX)
	{
		bgfx::destroy(bgfx::IndexBufferHandle{ m_indexBufferHandle });
		m_indexBufferHandle = UINT16_MAX;
	}

}

}