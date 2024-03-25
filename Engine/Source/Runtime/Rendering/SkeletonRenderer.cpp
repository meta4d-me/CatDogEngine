#include "SkeletonRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Rendering/Resources/ShaderResource.h"

namespace engine
{

namespace details
{

constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);

constexpr size_t indexTypeSize = sizeof(uint16_t);

cd::Vec3f CalculateBoneTranslate(const cd::Bone& bone, cd::Vec3f& translate, const cd::SceneDatabase* pSceneDatabase)
{
	const cd::Bone& parentBone = pSceneDatabase->GetBone(bone.GetParentID().Data());
	translate += parentBone.GetTransform().GetTranslation();
	if (0U != bone.GetParentID().Data())
	{
		CalculateBoneTranslate(parentBone, translate, pSceneDatabase);
	}
	return translate;
}

void TraverseBone(const cd::Bone& bone, const cd::SceneDatabase* pSceneDatabase, std::byte* currentDataPtr,
	std::byte* currentIndexPtr, uint32_t& vertexOffset, uint32_t& indexOffset)
{
	constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
	for (auto& child : bone.GetChildIDs())
	{
		const cd::Bone& currBone = pSceneDatabase->GetBone(child.Data());
		const cd::Bone& parent = pSceneDatabase->GetBone(currBone.GetParentID().Data());
		cd::Vec3f translate = currBone.GetOffset().GetTranslation();

		//const cd::Vec3f position = details::CalculateBoneTranslate(currBone, translate, pSceneDatabase);

		uint16_t parentID = currBone.GetParentID().Data();
		uint16_t currBoneID = currBone.GetID().Data();
		std::memcpy(&currentDataPtr[vertexOffset], translate.begin(), posDataSize);
		vertexOffset += posDataSize;
		std::memcpy(&currentIndexPtr[indexOffset], &parentID, indexTypeSize);
		indexOffset += static_cast<uint32_t>(indexTypeSize);
		std::memcpy(&currentIndexPtr[indexOffset], &currBoneID, indexTypeSize);
		indexOffset += static_cast<uint32_t>(indexTypeSize);

		TraverseBone(currBone, pSceneDatabase, currentDataPtr, currentIndexPtr, vertexOffset, indexOffset);
	}
}

}

void SkeletonRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("SkeletonProgram", "vs_AABB", "fs_AABB"));

	bgfx::setViewName(GetViewID(), "SkeletonRenderer");
}

void SkeletonRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void SkeletonRenderer::Build()
{
	const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();

	const uint32_t vertexCount = pSceneDatabase->GetBoneCount();
	if (0 == vertexCount)
	{
		return;
	}

	const cd::Bone& firstBone = pSceneDatabase->GetBone(0);
	if (0 != firstBone.GetID().Data())
	{
		CD_ENGINE_WARN("First BoneID is not 0");
		return;
	}

	bgfx::setTransform(cd::Matrix4x4::Identity().begin());
	cd::VertexFormat vertexFormat;
	vertexFormat.AddVertexAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);

	constexpr size_t indexTypeSize = sizeof(uint16_t);
	m_indexBuffer.resize((vertexCount - 1) * 2 * indexTypeSize);
	m_vertexBuffer.resize(vertexCount * vertexFormat.GetStride());
	uint32_t currentVertexOffset = 0U;
	uint32_t currentIndexOffset = 0U;
	std::byte* pCurrentVertexBuffer = m_vertexBuffer.data();
	const cd::Point& position = firstBone.GetTransform().GetTranslation();
	std::memcpy(&pCurrentVertexBuffer[currentVertexOffset], position.begin(), details::posDataSize);
	currentVertexOffset += details::posDataSize;

	details::TraverseBone(firstBone, pSceneDatabase, m_vertexBuffer.data(), m_indexBuffer.data(), currentVertexOffset, currentIndexOffset);
	bgfx::VertexLayout vertexLayout;
	VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexAttributeLayouts());
	m_boneVBH = bgfx::createVertexBuffer(bgfx::makeRef(m_vertexBuffer.data(), static_cast<uint32_t>(m_vertexBuffer.size())), vertexLayout).idx;
	m_boneIBH = bgfx::createIndexBuffer(bgfx::makeRef(m_indexBuffer.data(), static_cast<uint32_t>(m_indexBuffer.size())), 0U).idx;

}

void SkeletonRenderer::Render(float delataTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	for (Entity entity : m_pCurrentSceneWorld->GetAnimationEntities())
	{
		auto pAnimationComponent = m_pCurrentSceneWorld->GetAnimationComponent(entity);
		if (!pAnimationComponent)
		{
			continue;
		}
		if (!hasBuilt)
		{
			Build();
			hasBuilt = true;
		}

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ m_boneVBH });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ m_boneIBH });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;

		bgfx::setState(state);

		constexpr StringCrc programHandleIndex{ "SkeletonProgram" };
		GetRenderContext()->Submit(GetViewID(), programHandleIndex);
	}
	
}

}