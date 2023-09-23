#include "SkeletonRenderer.h"

#include "Core/StringCrc.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkinMeshComponent.h"
#include "RenderContext.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Window/Input.h"

namespace engine
{

namespace details
{

void CalculateChangeTranslate(const cd::Bone& bone, const cd::SceneDatabase* pSceneDatabase, engine::SkinMeshComponent* pSkinMeshComponent, const cd::Matrix4x4& changeBoneMatrix)
{
	for (auto& boneChild : bone.GetChildIDs())
	{
		pSkinMeshComponent->SetBoneChangeMatrix(boneChild.Data(), changeBoneMatrix);
		CalculateChangeTranslate(pSceneDatabase->GetBone(boneChild.Data()),pSceneDatabase, pSkinMeshComponent,changeBoneMatrix);
	}
}

void CalculateChangeRotation(const cd::Bone& bone, const cd::SceneDatabase* pSceneDatabase, engine::SkinMeshComponent* pSkinMeshComponent, const cd::Matrix4x4& changeBoneMatrix)
{
	for (auto& boneChild : bone.GetChildIDs())
	{
		const cd::Matrix4x4& changeRotationMatrix = bone.GetOffset().Inverse() * changeBoneMatrix * bone.GetOffset();
		pSkinMeshComponent->SetBoneChangeMatrix(boneChild.Data(), changeBoneMatrix);
		CalculateChangeRotation(pSceneDatabase->GetBone(boneChild.Data()), pSceneDatabase, pSkinMeshComponent, changeBoneMatrix);
	}
}

void CalculateTranslate(const cd::Bone& bone, const cd::SceneDatabase* pSceneDatabase, engine::SkinMeshComponent* pSkinMeshComponent)
{
	for (cd::BoneID boneID : bone.GetChildIDs())
	{
		const cd::Bone& childBone = pSceneDatabase->GetBone(boneID.Data());
		cd::Matrix4x4 matrix = pSkinMeshComponent->GetBoneMatrix(boneID.Data()).Inverse() * pSkinMeshComponent->GetBoneChangeMatrix(boneID.Data());
		pSkinMeshComponent->SetBoneChangeMatrix(childBone.GetID().Data(), matrix);
		CalculateTranslate(childBone, pSceneDatabase, pSkinMeshComponent);
	}
}

void CalculateBoneDeltaTransform(const cd::Bone& bone,uint32_t& time, const cd::SceneDatabase* pSceneDatabase, engine::SkinMeshComponent* pSkinMeshComponent,  cd::Matrix4x4& parentBoneTransform)
{
	cd::Matrix4x4 boneLocalTransform = bone.GetTransform().GetMatrix();
	cd::Matrix4x4 parentWorldMatrix = cd::Matrix4x4::Identity();
	cd::Quaternion parentRotationInserve = cd::Quaternion::Identity();
	if (bone.GetID().Data() != 0)
	{
		const cd::Bone& parentBone = pSceneDatabase->GetBone(bone.GetParentID().Data());
		parentWorldMatrix = parentBone.GetOffset().Inverse();
		parentRotationInserve = cd::Quaternion::FromMatrix(parentBone.GetOffset().GetRotation());

	}
	cd::Matrix4x4 globalTransform = parentWorldMatrix * boneLocalTransform;
	if (const cd::Track* pTrack = pSceneDatabase->GetTrackByName(bone.GetName()))
	{
		//for (uint32_t index = 0; index < pTrack->GetTranslationKeyCount(); index++)
		//{
		if (time < pTrack->GetRotationKeyCount())
		{
			const char* name = bone.GetName();
			const auto& transKey = pTrack->GetTranslationKeys()[1];
			cd::Vec3f translation = transKey.GetValue();
		
			
			const auto& rotationKey= pTrack->GetRotationKeys()[1];
			cd::Quaternion rotation = rotationKey.GetValue();

			cd::Vec3f sourceTranslation = bone.GetTransform().GetTranslation();
			cd::Vec3f deltaTranslation = translation - sourceTranslation;

			cd::Quaternion sourcerotation = bone.GetTransform().GetRotation();
			cd::Quaternion deltaRotation = rotation * sourcerotation;
			deltaRotation = deltaRotation * parentRotationInserve;

			cd::Transform changeTransform = cd::Transform(deltaTranslation, deltaRotation, cd::Vec3f(1.0f, 1.0f, 1.0f));
			cd::Matrix4x4 globalTransform = parentBoneTransform * changeTransform.GetMatrix();
			pSkinMeshComponent->SetBoneChangeMatrix(bone.GetID().Data(), globalTransform);
			parentBoneTransform = globalTransform;

		}
		else
		{
			time = 0;
		}


	}


}

}

void SkeletonRenderer::Init()
{
	GetRenderContext()->CreateProgram("SkeletonProgram", "vs_skeleton.bin", "fs_AABB.bin");
	bgfx::setViewName(GetViewID(), "SkeletonRenderer");
}

void SkeletonRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void SkeletonRenderer::Render(float delataTime)
{
	const cd::SceneDatabase* pSceneDatabase = m_pCurrentSceneWorld->GetSceneDatabase();
	for (Entity entity : m_pCurrentSceneWorld->GetAnimationEntities())
	{
		auto pSkinMeshComponent = m_pCurrentSceneWorld->GetSkinMeshComponent(entity);
		if (!pSkinMeshComponent)
		{
			continue;
		}

		if (Input::Get().IsKeyPressed(KeyCode::g))
		{
			float dx = Input::Get().GetMousePositionOffsetX() * delataTime * 10.0f;
			float dy = -Input::Get().GetMousePositionOffsetY() * delataTime * 10.0f;
			cd::Transform transform = cd::Transform::Identity();
			transform.SetTranslation(cd::Vec3f(dx, dy, 0));
			const uint32_t changeBoneIndex = m_pCurrentSceneWorld->GetSelectedBoneID().Data();
			const cd::Bone& bone = pSceneDatabase->GetBone(changeBoneIndex);
			//const cd::Matrix4x4& changeBoneMatrix = pSkinMeshComponent->GetBoneChangeMatrix(changeBoneIndex);
			const cd::Matrix4x4& changeBoneMatrix = transform.GetMatrix();
			pSkinMeshComponent->SetBoneChangeMatrix(changeBoneIndex,changeBoneMatrix);
			details::CalculateChangeTranslate(bone, pSceneDatabase, pSkinMeshComponent,changeBoneMatrix);
			pSkinMeshComponent->ResetChangeBoneIndex();
		}

		if (Input::Get().IsKeyPressed(KeyCode::r))
		{
			float dx = Input::Get().GetMousePositionOffsetX() * delataTime * 30.0f;
			float dy = -Input::Get().GetMousePositionOffsetY() * delataTime * 30.0f;
			cd::Quaternion rotation = cd::Quaternion::FromPitchYawRoll(dy, 0.0f, 0.0f);
			cd::Transform transform = cd::Transform::Identity();
			const uint32_t changeBoneIndex = m_pCurrentSceneWorld->GetSelectedBoneID().Data();
			const cd::Bone& bone = pSceneDatabase->GetBone(changeBoneIndex);
			const cd::Bone& parent = pSceneDatabase->GetBone(bone.GetParentID().Data());
			transform.SetRotation(rotation);
			CD_INFO(transform.GetRotation());
			const cd::Matrix4x4& changeBoneMatrix = parent.GetOffset().Inverse() * transform.GetMatrix() * parent.GetOffset();
			pSkinMeshComponent->SetBoneChangeMatrix(changeBoneIndex, changeBoneMatrix);
			details::CalculateChangeRotation(bone, pSceneDatabase, pSkinMeshComponent, changeBoneMatrix);
			pSkinMeshComponent->ResetChangeBoneIndex();
		}
		//details::CalculateTranslate(pSceneDatabase->GetBone(0), pSceneDatabase, pSkinMeshComponent);
		cd::Matrix4x4 dd = cd::Matrix4x4::Identity();
		//details::CalculateBoneDeltaTransform(pSceneDatabase->GetBone(0), m_time, pSceneDatabase, pSkinMeshComponent, dd);
		//details::CalculateBoneDeltaTransform(pSceneDatabase->GetBone(1), m_time, pSceneDatabase, pSkinMeshComponent, dd);
		//details::CalculateBoneDeltaTransform(pSceneDatabase->GetBone(2), m_time, pSceneDatabase, pSkinMeshComponent, dd);

		m_time++;
		bgfx::setUniform(bgfx::UniformHandle{ pSkinMeshComponent->GetBoneMatrixsUniform() }, pSkinMeshComponent->GetBoneChangeMatrices().data(), static_cast<uint16_t>(pSkinMeshComponent->GetBoneChangeMatrices().size()));
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pSkinMeshComponent->GetBoneVBH() });
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pSkinMeshComponent->GetBoneIBH() });

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_PT_LINES;

		bgfx::setState(state);
		constexpr StringCrc SkeletonProgram("SkeletonProgram");
		bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(SkeletonProgram));
	}
	
}

}