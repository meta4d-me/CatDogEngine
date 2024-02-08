#include "BlendShapeRenderer.h"

#include "ECWorld/BlendShapeComponent.h"
#include "ECWorld/CameraComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/SkyComponent.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "LightUniforms.h"
#include "Material/ShaderSchema.h"
#include "Math/Transform.hpp"
#include "RenderContext.h"
#include "Scene/Texture.h"
#include "U_IBL.sh"
#include "U_AtmophericScattering.sh"
#include "U_BlendShape.sh"

namespace engine
{

namespace
{

constexpr const char* morphCountVertexCount = "u_morphCount_vertexCount";
constexpr const char* changedWeight = "u_changedWeight";

constexpr const char *BlendShapeWeightsProgram = "BlendShapeWeightsProgram";
constexpr const char *BlendShapeWeightPosProgram = "BlendShapeWeightPosProgram";
constexpr const char *BlendShapeFinalPosProgram = "BlendShapeFinalPosProgram";
constexpr const char *BlendShapeUpdatePosProgram = "BlendShapeUpdatePosProgram";

constexpr StringCrc BlendShapeWeightsProgramCrc = StringCrc("BlendShapeWeightsProgram");
constexpr StringCrc BlendShapeWeightPosProgramCrc = StringCrc("BlendShapeWeightPosProgram");
constexpr StringCrc BlendShapeFinalPosProgramCrc = StringCrc("BlendShapeFinalPosProgram");
constexpr StringCrc BlendShapeUpdatePosProgramCrc = StringCrc("BlendShapeUpdatePosProgram");

}

void BlendShapeRenderer::Init()
{
	GetRenderContext()->RegisterShaderProgram(BlendShapeWeightsProgramCrc, { "cs_blendshape_weights" });
	GetRenderContext()->RegisterShaderProgram(BlendShapeWeightPosProgramCrc, { "cs_blendshape_weight_pos" });
	GetRenderContext()->RegisterShaderProgram(BlendShapeFinalPosProgramCrc, { "cs_blendshape_final_pos" });
	GetRenderContext()->RegisterShaderProgram(BlendShapeUpdatePosProgramCrc, { "cs_blendshape_update_pos" });

	bgfx::setViewName(GetViewID(), "BlendShapeRenderer");
}

void BlendShapeRenderer::Warmup()
{
	GetRenderContext()->CreateUniform(morphCountVertexCount, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(changedWeight, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->UploadShaderProgram(BlendShapeWeightsProgram);
	GetRenderContext()->UploadShaderProgram(BlendShapeWeightPosProgram);
	GetRenderContext()->UploadShaderProgram(BlendShapeFinalPosProgram);
	GetRenderContext()->UploadShaderProgram(BlendShapeUpdatePosProgram);
}

void BlendShapeRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void BlendShapeRenderer::Render(float deltaTime)
{
	for (Entity entity : m_pCurrentSceneWorld->GetBlendShapeEntities())
	{
		// No blend shape?
		BlendShapeComponent* pBlendShapeComponent = m_pCurrentSceneWorld->GetBlendShapeComponent(entity);
		if (!pBlendShapeComponent)
		{
			continue;
		}

		uint16_t viewId = GetViewID();

		// Compute Blend Shape
		if (pBlendShapeComponent->IsDirty())
		{
			bgfx::setBuffer(BS_ALL_MORPH_VERTEX_ID_STAGE, bgfx::IndexBufferHandle{pBlendShapeComponent->GetAllMorphVertexIDIB()}, bgfx::Access::Read);
			bgfx::setBuffer(BS_ACTIVE_MORPH_DATA_STAGE, bgfx::DynamicIndexBufferHandle{pBlendShapeComponent->GetActiveMorphOffestLengthWeightIB()}, bgfx::Access::Read);
			bgfx::setBuffer(BS_FINAL_MORPH_AFFECTED_STAGE, bgfx::DynamicVertexBufferHandle{pBlendShapeComponent->GetFinalMorphAffectedVB()}, bgfx::Access::ReadWrite);
			constexpr StringCrc morphCountVertexCountCrc(morphCountVertexCount);
			cd::Vec4f morphCount{ static_cast<float>(pBlendShapeComponent->GetActiveMorphCount()), static_cast<float>(pBlendShapeComponent->GetMeshVertexCount()), 0, 0};
			GetRenderContext()->FillUniform(morphCountVertexCountCrc, &morphCount, 1);
			GetRenderContext()->Dispatch(viewId, BlendShapeWeightsProgram, 1U, 1U, 1U);

			bgfx::setBuffer(BS_MORPH_AFFECTED_STAGE, bgfx::VertexBufferHandle{pBlendShapeComponent->GetMorphAffectedVB()}, bgfx::Access::Read);
			bgfx::setBuffer(BS_FINAL_MORPH_AFFECTED_STAGE, bgfx::DynamicVertexBufferHandle{pBlendShapeComponent->GetFinalMorphAffectedVB()}, bgfx::Access::ReadWrite);
			GetRenderContext()->FillUniform(morphCountVertexCountCrc, &morphCount, 1);
			GetRenderContext()->Dispatch(viewId, BlendShapeWeightPosProgram, 1U, 1U, 1U);

			bgfx::setBuffer(BS_FINAL_MORPH_AFFECTED_STAGE, bgfx::DynamicVertexBufferHandle{pBlendShapeComponent->GetFinalMorphAffectedVB()}, bgfx::Access::ReadWrite);
			bgfx::setBuffer(BS_ALL_MORPH_VERTEX_ID_STAGE, bgfx::IndexBufferHandle{pBlendShapeComponent->GetAllMorphVertexIDIB()}, bgfx::Access::Read);
			bgfx::setBuffer(BS_ACTIVE_MORPH_DATA_STAGE, bgfx::DynamicIndexBufferHandle{pBlendShapeComponent->GetActiveMorphOffestLengthWeightIB()}, bgfx::Access::Read);
			GetRenderContext()->FillUniform(morphCountVertexCountCrc, &morphCount, 1);
			GetRenderContext()->Dispatch(viewId, BlendShapeFinalPosProgram, 1U, 1U, 1U);

			pBlendShapeComponent->SetDirty(false);
		}

		if (pBlendShapeComponent->NeedUpdate()) 
		{
			pBlendShapeComponent->UpdateChanged();
			bgfx::setBuffer(BS_MORPH_AFFECTED_STAGE, bgfx::VertexBufferHandle{pBlendShapeComponent->GetMorphAffectedVB()}, bgfx::Access::Read);
			bgfx::setBuffer(BS_ALL_MORPH_VERTEX_ID_STAGE, bgfx::IndexBufferHandle{pBlendShapeComponent->GetAllMorphVertexIDIB()}, bgfx::Access::Read);
			bgfx::setBuffer(BS_ACTIVE_MORPH_DATA_STAGE, bgfx::DynamicIndexBufferHandle{pBlendShapeComponent->GetActiveMorphOffestLengthWeightIB()}, bgfx::Access::Read);
			bgfx::setBuffer(BS_FINAL_MORPH_AFFECTED_STAGE, bgfx::DynamicVertexBufferHandle{pBlendShapeComponent->GetFinalMorphAffectedVB()}, bgfx::Access::ReadWrite);
			bgfx::setBuffer(BS_CHANGED_MORPH_INDEX_STAGE, bgfx::DynamicIndexBufferHandle{pBlendShapeComponent->GetChangedMorphIndexIB()}, bgfx::Access::Read);
			//constexpr StringCrc changedWeightCrc(changedWeight);
			//cd::Vec4f changedWeightData = cd::Vec4f{ static_cast<float>(pBlendShapeComponent->GetUpdatedWeight()),0,0,0 };
			//GetRenderContext()->FillUniform(changedWeightCrc, &changedWeightData, 1);
			GetRenderContext()->Dispatch(viewId, BlendShapeUpdatePosProgram, 1U, 1U, 1U);

			pBlendShapeComponent->ClearNeedUpdate();
		}
	}
}

}