#include "MeshResource.h"

#include "Log/Log.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Utilities/MeshUtils.hpp"

namespace details
{

uint16_t SubmitVertexBuffer(const engine::MeshResource::VertexBuffer& vertexBuffer, const cd::VertexFormat& vertexFormat)
{
	bgfx::VertexLayout vertexLayout;
	engine::VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexAttributeLayouts());
	const bgfx::Memory* pVertexBufferRef = bgfx::makeRef(vertexBuffer.data(), static_cast<uint32_t>(vertexBuffer.size()));
	bgfx::VertexBufferHandle vertexBufferHandle = bgfx::createVertexBuffer(pVertexBufferRef, vertexLayout);
	assert(bgfx::isValid(vertexBufferHandle));
	return vertexBufferHandle.idx;
}

enum class IndexBufferType
{
	Static,
	Dynamic
};

template<IndexBufferType IBT = IndexBufferType::Static>
uint16_t SubmitIndexBuffer(const engine::MeshResource::IndexBuffer& indexBuffer, bool useU16Index = false)
{
	const bgfx::Memory* pIndexBufferRef = bgfx::makeRef(indexBuffer.data(), static_cast<uint32_t>(indexBuffer.size()));
	if constexpr (IndexBufferType::Static == IBT)
	{
		bgfx::IndexBufferHandle indexBufferHandle = bgfx::createIndexBuffer(pIndexBufferRef, useU16Index ? 0U : BGFX_BUFFER_INDEX32);
		assert(bgfx::isValid(indexBufferHandle));
		return indexBufferHandle.idx;
	}
	else if constexpr (IndexBufferType::Dynamic == IBT)
	{
		bgfx::DynamicIndexBufferHandle indexBufferHandle = bgfx::createDynamicIndexBuffer(pIndexBufferRef, useU16Index ? 0U : BGFX_BUFFER_INDEX32);
		assert(bgfx::isValid(indexBufferHandle));
		return indexBufferHandle.idx;
	}
	else
	{
		static_assert("Unsupported IndexBufferType.");
	}
};

}

namespace engine
{

MeshResource::MeshResource() = default;

MeshResource::~MeshResource()
{
	// Collect garbage intermediatly.
	SetStatus(ResourceStatus::Garbage);
	Update();
}

uint16_t MeshResource::GetVertexBufferHandle() const
{
	return m_vertexBufferHandle;
}

uint16_t MeshResource::GetIndexBufferHandle(uint32_t index) const
{
	return m_indexBufferHandles[index];
}

void MeshResource::SetMeshAsset(const cd::Mesh* pMeshAsset)
{
	if (pMeshAsset)
	{
		if (ResourceStatus::Loading == GetStatus())
		{
			m_pMeshAsset = pMeshAsset;
			m_vertexCount = pMeshAsset->GetVertexCount();
			m_polygonCount = pMeshAsset->GetPolygonCount();
			m_polygonGroupCount = pMeshAsset->GetPolygonGroupCount();
			SetStatus(ResourceStatus::Loaded);
		}
	}
	else
	{
		if (ResourceStatus::Building != GetStatus())
		{
			// Cleanup asset reference. Can't rebuild data such as UpdateVertexFormat after this operation.
			m_pMeshAsset = nullptr;
		}
	}
}

void MeshResource::UpdateVertexFormat(const cd::VertexFormat& vertexFormat)
{
	if (!m_pMeshAsset)
	{
		return;
	}

	for (const auto& targetLayout : vertexFormat.GetVertexAttributeLayouts())
	{
		const auto* pSourceLayout = m_currentVertexFormat.GetVertexAttributeLayout(targetLayout.vertexAttributeType);
		if (pSourceLayout)
		{
			// If not same, please consider (one more vertex buffer memory costs) vs (keep shader codes consistent in vertex format usages).
			assert(pSourceLayout->attributeCount == targetLayout.attributeCount);
			assert(pSourceLayout->vertexAttributeType == targetLayout.vertexAttributeType);
		}
		else
		{
			if (m_pMeshAsset->GetVertexFormat().Contains(targetLayout.vertexAttributeType))
			{
				// CPU data needs to rebuild and send to gpu again.
				SetStatus(ResourceStatus::Building);
				m_currentVertexFormat.AddVertexAttributeLayout(targetLayout);
				DestroyVertexBufferHandle();
			}
			else
			{
				assert("Mesh doesn't contain required vertex attributes.");
			}
		}
	}
}

void MeshResource::Update()
{
	switch (GetStatus())
	{
	case ResourceStatus::Building:
	{
		BuildVertexBuffer();
		BuildIndexBuffer();
		SetStatus(ResourceStatus::Built);
		break;
	}
	case ResourceStatus::Built:
	{
		SubmitVertexBuffer();
		SubmitIndexBuffer();
		SetStatus(ResourceStatus::Ready);
		m_recycleCount = 0U;
		break;
	}
	case ResourceStatus::Ready:
	{
		// Release CPU data later to save memory.
		constexpr uint32_t recycleDelayFrames = 30U;
		if (m_recycleCount++ >= recycleDelayFrames)
		{
			m_vertexBuffer.clear();
			m_indexBuffers.clear();
			SetStatus(ResourceStatus::Optimized);
			m_recycleCount = 0U;
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

bool MeshResource::BuildVertexBuffer()
{
	assert(m_pMeshAsset && m_vertexCount > 0U);

	std::optional<cd::VertexBuffer> optVertexBuffer = cd::BuildVertexBufferForStaticMesh(*m_pMeshAsset, m_currentVertexFormat);
	if (!optVertexBuffer.has_value())
	{
		CD_ERROR("Failed to build mesh vertex buffer.");
		return false;
	}

	// VertexBuffer may rebuild based on VertexFormat update so clear it explicitly.
	m_vertexBuffer.clear();
	
	m_vertexBuffer = cd::MoveTemp(optVertexBuffer.value());
	
	return true;
}

bool MeshResource::BuildIndexBuffer()
{
	assert(m_pMeshAsset && m_polygonCount > 0U && m_polygonGroupCount > 0U);

	// IndexBuffer seems not necessary to rebuild many times so we only rebuild it when detect empty data.
	if (!m_indexBuffers.empty())
	{
		bool rebuild = false;
		for (const auto& indexBuffer : m_indexBuffers)
		{
			if (indexBuffer.empty())
			{
				rebuild = true;
				break;
			}
		}

		if (!rebuild)
		{
			return true;
		}
	}

	bool result = true;
	m_indexBuffers.resize(m_polygonGroupCount);
	for (uint32_t polygonGroupIndex = 0U; polygonGroupIndex < m_polygonGroupCount; ++polygonGroupIndex)
	{
		std::optional<cd::IndexBuffer> optIndexBuffer = cd::BuildIndexBufferesForPolygonGroup(*m_pMeshAsset, polygonGroupIndex);
		if (optIndexBuffer.has_value())
		{
			m_indexBuffers[polygonGroupIndex] = cd::MoveTemp(optIndexBuffer.value());
		}
		else
		{
			result = false;
			CD_ERROR("Failed to build mesh index buffer.");
		}
	}

	return result;
}

void MeshResource::SubmitVertexBuffer()
{
	if (m_vertexBufferHandle != UINT16_MAX)
	{
		return;
	}

	m_vertexBufferHandle = details::SubmitVertexBuffer(m_vertexBuffer, m_currentVertexFormat);
}

void MeshResource::SubmitIndexBuffer()
{
	if (!m_indexBufferHandles.empty())
	{
		bool submit = false;
		for (uint16_t indexBufferHandle : m_indexBufferHandles)
		{
			if (indexBufferHandle == UINT16_MAX)
			{
				submit = true;
				break;
			}
		}

		if (!submit)
		{
			return;
		}
	}

	size_t indexBufferCount = m_indexBuffers.size();
	assert(indexBufferCount > 0);
	m_indexBufferHandles.resize(indexBufferCount, UINT16_MAX);

	const bool useU16Index = m_vertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
	for (size_t bufferIndex = 0; bufferIndex < indexBufferCount; ++bufferIndex)
	{
		const auto& indexBuffer = m_indexBuffers[bufferIndex];
		assert(!indexBuffer.empty());
		m_indexBufferHandles[bufferIndex] = details::SubmitIndexBuffer(indexBuffer, useU16Index);
	}
}

void MeshResource::DestroyVertexBufferHandle()
{
	if (m_vertexBufferHandle != UINT16_MAX)
	{
		bgfx::destroy(bgfx::VertexBufferHandle{ m_vertexBufferHandle });
		m_vertexBufferHandle = UINT16_MAX;
	}
}

void MeshResource::DestroyIndexBufferHandle()
{
	for (uint16_t indexBufferHandle : m_indexBufferHandles)
	{
		if (indexBufferHandle != UINT16_MAX)
		{
			bgfx::destroy(bgfx::IndexBufferHandle{ indexBufferHandle });
			indexBufferHandle = UINT16_MAX;
		}
	}

	m_indexBufferHandles.clear();
}

}