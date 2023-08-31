#include "SkeletonRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "RenderContext.h"
#include "Rendering/Utility/VertexLayoutUtility.h"

namespace engine
{

namespace details
{
void TraverseBone(const cd::Bone& bone, const cd::SceneDatabase* pSceneDatabase, std::byte* currentDataPtr, 
	std::byte* currentIndexPtr, uint32_t indexTypeSize, uint32_t currentDataSize, uint32_t currentIndexSize)
{
	constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);

	for (auto& child : bone.GetChildIDs())
	{
		const cd::Bone& currBone = pSceneDatabase->GetBone(child.Data());
		const cd::Bone& parent = pSceneDatabase->GetBone(currBone.GetParentID().Data());
		const cd::Point& position = currBone.GetOffset().GetTranslation();
		uint32_t parentID = currBone.GetParentID().Data();
		uint32_t currBoneID = currBone.GetID().Data();
		std::memcpy(&currentDataPtr[currentDataSize], position.Begin(), posDataSize);
		std::memcpy(&currentIndexPtr[currentIndexSize], &parentID, indexTypeSize);
		currentIndexSize++;
		std::memcpy(&currentIndexPtr[currentIndexSize], &currBoneID, indexTypeSize);
		currentIndexSize++;

		currentDataSize += posDataSize;
		TraverseBone(currBone, pSceneDatabase, currentDataPtr, currentIndexPtr, indexTypeSize, currentDataSize, currentIndexSize);
	}
}
}

void SkeletonRenderer::Init()
{
	GetRenderContext()->CreateProgram("SkeletonProgram", "vs_AABB.bin", "fs_AABB.bin");
	bgfx::setViewName(GetViewID(), "SkeletonRenderer");
}

void SkeletonRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void SkeletonRenderer::Render(float delataTime)
{
	for (Entity entity : m_pCurrentSceneWorld->GetAnimationEntities())
	{
		auto pAnimationComponent = m_pCurrentSceneWorld->GetAnimationComponent(entity);
		if (!pAnimationComponent)
		{
			continue;
		}
	
		const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
		const cd::Bone& firstBone = pSceneDatabase->GetBone(0);

		const uint32_t vertexCount = pSceneDatabase->GetBoneCount();
		float translation[16] = {
	1.0f, 0.0f, 0.0f, 20.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
		};
		bgfx::setTransform(translation);
		cd::VertexFormat vertexFormat;
		vertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::AttributeValueType::Float, 3);

		bool useU16Index = vertexCount <= static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()) + 1U;
		uint32_t indexTypeSize = useU16Index ? sizeof(uint16_t) : sizeof(uint32_t);
		m_indexBuffer.resize((vertexCount - 1) * 2 * indexTypeSize);

		m_vertexBuffer.resize(vertexCount * vertexFormat.GetStride());
		uint32_t currentDataSize = 0U;
		uint32_t currentIndexSize = 0U;
		auto currentDataPtr = m_vertexBuffer.data();
		auto currentIndexPtr = m_indexBuffer.data();
		constexpr uint32_t posDataSize = cd::Point::Size * sizeof(cd::Point::ValueType);
		const cd::Point& position = firstBone.GetOffset().GetTranslation();
		std::memcpy(&currentDataPtr[currentDataSize], position.Begin(), posDataSize);
		currentDataSize += 1;
		details::TraverseBone(firstBone, pSceneDatabase, currentDataPtr, currentIndexPtr, indexTypeSize, currentDataSize, currentIndexSize);

		bgfx::VertexLayout vertexLayout;
		VertexLayoutUtility::CreateVertexLayout(vertexLayout, vertexFormat.GetVertexLayout());
		m_boneVBH = bgfx::createVertexBuffer(bgfx::makeRef(m_vertexBuffer.data(), static_cast<uint32_t>(m_vertexBuffer.size())), vertexLayout).idx;
		m_boneIBH = bgfx::createIndexBuffer(bgfx::makeRef(m_indexBuffer.data(), static_cast<uint32_t>(m_indexBuffer.size())), 0U).idx;

		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ m_boneVBH });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ m_boneIBH });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;

		bgfx::setState(state);
		constexpr StringCrc SkeletonProgram("SkeletonProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(SkeletonProgram));
	}

}
}