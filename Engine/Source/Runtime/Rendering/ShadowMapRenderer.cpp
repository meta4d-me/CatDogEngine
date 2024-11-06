#include "ShadowMapRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/LightComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "LightUniforms.h"
#include "Material/ShaderSchema.h"
#include "Math/Transform.hpp"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/MeshResource.h"
#include "Rendering/Resources/ShaderResource.h"

#include <string>

namespace engine
{

namespace
{

constexpr const char* cameraPos                   = "u_cameraPos";
constexpr const char* lightCountAndStride         = "u_lightCountAndStride";
constexpr const char* lightParams                 = "u_lightParams";
constexpr const char* lightPosAndFarPlane         = "u_lightWorldPos_farPlane";
constexpr const char* lightDir                    = "u_LightDir";
constexpr const char* heightOffsetAndshadowLength = "u_HeightOffsetAndshadowLength";

constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr uint64_t defaultRenderingState = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
constexpr uint64_t depthBufferFlags = BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL;
constexpr uint64_t linearDepthBufferFlags = BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

}

void ShadowMapRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ShadowMapProgram", "vs_shadowMap", "fs_shadowMap"));
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("LinearShadowMapProgram", "vs_shadowMap", "fs_shadowMap_linear"));

	for (int lightIndex = 0; lightIndex < shadowLightMaxNum; lightIndex++)
	{
		for (int mapId = 0; mapId < shadowTexturePassMaxNum; mapId++)
		{
			m_renderPassID[lightIndex * shadowTexturePassMaxNum + mapId] = GetRenderContext()->CreateView();
			bgfx::setViewName(m_renderPassID[lightIndex * shadowTexturePassMaxNum + mapId], "ShadowMapRenderer");
		}
	}

	GetRenderContext()->CreateUniform(lightPosAndFarPlane, bgfx::UniformType::Vec4, 1);
}

void ShadowMapRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	//UpdateViewRenderTarget(); //in Render()
}

void ShadowMapRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

	// TODO : Remove it. If every renderer need to submit camera related uniform, it should be done not inside Renderer class.
	const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();

	// Submit uniform values : light settings
	auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();

	if (!lightEntities.empty())
	{
		// camera 
		CameraComponent* pMainCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
		const cd::Matrix4x4 camView = pMainCameraComponent->GetViewMatrix();
		const cd::Matrix4x4 camProj = pMainCameraComponent->GetProjectionMatrix();
		const cd::Matrix4x4 invCamViewProj = (camProj * camView).Inverse();
		bool ndcDepthMinusOneToOne = cd::NDCDepth::MinusOneToOne == pMainCameraComponent->GetNDCDepth();

		// lambda : unproject ndc sapce coordinates into world space 
		auto UnProject = [&invCamViewProj](const cd::Vec4f ndcCorner)->cd::Point
		{
			cd::Vec4f worldPos = invCamViewProj * ndcCorner;
			return worldPos.xyz() / worldPos.w();
		};

		uint16_t shadowNum = 0U;
		for (auto lightEntity : lightEntities)
		{
			LightComponent* lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntity);

			// Non-shadow-casting lights(include area lights) are excluded
			if (!lightComponent->IsCastShadow())
			{
				//continue;
			}

			// Render shadow map
			switch (lightComponent->GetType())
			{
			case cd::LightType::Directional:
			{
				uint16_t cascadeNum = lightComponent->GetCascadeNum();
				// Initialize(if not initialized) frame buffer
				if (!lightComponent->IsShadowMapFBsValid())
				{
					lightComponent->ClearShadowMapFBs();	
					for (int i = 0; i < cascadeNum; ++i)
					{
						uint16_t shadowMapFB = bgfx::createFrameBuffer(lightComponent->GetShadowMapSize(),
							lightComponent->GetShadowMapSize(), bgfx::TextureFormat::D32F).idx;
						lightComponent->AddShadowMapFB(shadowMapFB);
					}
				}

				// Compute the split distances based on the partitioning mode
				float CascadeSplits[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				float MinDistance = 0.0f;		// if this changed, UnProjection need changed
				float MaxDistance = 1.0f;	// if this changed, UnProjection need changed
				
				CascadePartitionMode cascadePatitionMode = lightComponent->GetCascadePartitionMode();
				if (CascadePartitionMode::Manual == cascadePatitionMode)
				{
					
				}
				else if (CascadePartitionMode::Logarithmic == cascadePatitionMode
					|| CascadePartitionMode::PSSM == cascadePatitionMode)
				{
					float lambda = 1.0f;
					if (CascadePartitionMode::PSSM == cascadePatitionMode){}
						lambda = 0.5;// TODO : user edit

					float nearClip = pMainCameraComponent->GetNearPlane();
					float farClip = pMainCameraComponent->GetFarPlane();
					float clipRange = farClip - nearClip;

					float minZ = nearClip + MinDistance * clipRange;
					float maxZ = nearClip + MaxDistance * clipRange;

					float range = maxZ - minZ;
					float ratio = maxZ / minZ;

					for (uint16_t i = 0; i < cascadeNum; ++i)
					{
						float p = (i + 1) / static_cast<float>(cascadeNum);
						float log = minZ * std::pow(ratio, p);
						float uniform = minZ + range * p;
						float d = lambda * (log - uniform) + uniform;
						CascadeSplits[i] = (d - nearClip) / clipRange;
					}
				}

				// Compute the frustum according to ndc depth of different graphic backends
				cd::Direction lightDirection = lightComponent->GetDirection();
				std::vector<cd::Point> frustumCorners = ndcDepthMinusOneToOne ? 
					std::vector<cd::Point>{		 // Depth [-1, 1]
						UnProject(cd::Vec4f(-1, -1,  -1, 1)),	UnProject(cd::Vec4f(-1, -1, 1, 1)),	// lower-left near and far 
						UnProject(cd::Vec4f(-1,  1,  -1, 1)),	UnProject(cd::Vec4f(-1,  1, 1, 1)),	// upper-left near and far 
						UnProject(cd::Vec4f( 1, -1,  -1, 1)),	UnProject(cd::Vec4f( 1, -1, 1, 1)),	//	lower-right near and far 
						UnProject(cd::Vec4f( 1,  1,  -1, 1)),	UnProject(cd::Vec4f( 1,  1, 1, 1))		// upper-right near and far
					} :
					std::vector<cd::Point>{		// Depth [0, 1]
						UnProject(cd::Vec4f(-1, -1,  0, 1)),	UnProject(cd::Vec4f(-1, -1, 1, 1)),	// lower-left near and far 
						UnProject(cd::Vec4f(-1,  1,  0, 1)),	UnProject(cd::Vec4f(-1,  1, 1, 1)),	// upper-left near and far 
						UnProject(cd::Vec4f( 1, -1,  0, 1)),	UnProject(cd::Vec4f( 1, -1, 1, 1)),	//	lower-right near and far 
						UnProject(cd::Vec4f( 1,  1,  0, 1)),	UnProject(cd::Vec4f( 1,  1, 1, 1))		// upper-right near and far
					};

				// Set cascade split dividing values for choosing cascade level in world renderer
				lightComponent->SetComputedCascadeSplit(&CascadeSplits[0]);
				lightComponent->ClearLightViewProjMatrix();
				for (uint16_t cascadeIndex = 0; cascadeIndex < cascadeNum; ++cascadeIndex)
				{
					// Compute every light view and every orthographic projection matrices for each cascade
					cd::Point cascadeFrustum[8];
					float nearSplit = cascadeIndex == 0 ? MinDistance : CascadeSplits[cascadeIndex - 1];
					float farSplit = CascadeSplits[cascadeIndex];
					for (uint16_t cornerPairIdx = 0; cornerPairIdx < 4; ++cornerPairIdx)
					{
						cascadeFrustum[2*cornerPairIdx]		=	frustumCorners[2*cornerPairIdx] * (1 - nearSplit) 
							+ frustumCorners[2*cornerPairIdx+1] * nearSplit;
						cascadeFrustum[2*cornerPairIdx+1]	=  frustumCorners[2*cornerPairIdx] * (1 - farSplit)		
							+ frustumCorners[2*cornerPairIdx+1] * farSplit;
					}
					
					cd::Point cascadeFrustumCenter = cd::Point(0, 0, 0);
					for (const auto& corner : cascadeFrustum)
					{
						cascadeFrustumCenter += corner;
					}
					cascadeFrustumCenter /= 8.0;
					
					cd::Matrix4x4 lightView = cd::Matrix4x4::LookAt<cd::Handedness::Left>(cascadeFrustumCenter, 
						cascadeFrustumCenter + lightDirection, cd::Vec3f(0.0f, 1.0f, 0.0f));

					float minX = std::numeric_limits<float>::max();
					float maxX = std::numeric_limits<float>::lowest();
					float minY = std::numeric_limits<float>::max();
					float maxY = std::numeric_limits<float>::lowest();
					float minZ = std::numeric_limits<float>::max();
					float maxZ = std::numeric_limits<float>::lowest();
					for (const auto& corner : cascadeFrustum)
					{
						const auto lightSpaceCorner = lightView * cd::Vec4f(corner.x(), corner.y(), corner.z(), 1.0);
						minX = std::min(minX, lightSpaceCorner.x());
						maxX = std::max(maxX, lightSpaceCorner.x());
						minY = std::min(minY, lightSpaceCorner.y());
						maxY = std::max(maxY, lightSpaceCorner.y());
						minZ = std::min(minZ, lightSpaceCorner.z());
						maxZ = std::max(maxZ, lightSpaceCorner.z());
					}
					cd::Matrix4x4 lightProjection = cd::Matrix4x4::Orthographic(minX, maxX, maxY, minY, minZ, maxZ,
						0, ndcDepthMinusOneToOne);

					// Settings
					bgfx::setState(defaultRenderingState);

					uint16_t viewId = m_renderPassID[shadowNum * shadowTexturePassMaxNum + cascadeIndex];
					bgfx::setViewRect(viewId, 0, 0, lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize());
					bgfx::setViewFrameBuffer(viewId, static_cast<bgfx::FrameBufferHandle>(lightComponent->GetShadowMapFBs().at(cascadeIndex)));
					bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH, 0xffffffff, 1.0f, 0);
					bgfx::setViewTransform(viewId, lightView.begin(), lightProjection.begin());

					// Set transform for projecting coordinates to light space in wolrd renderer 
					cd::Matrix4x4 lightCSMViewProj = lightProjection * lightView;
					lightComponent->AddLightViewProjMatrix(lightCSMViewProj);

					// Submit draw call (TODO : one pass MRT
					for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
					{
						// No mesh attached?
						StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
						if (!pMeshComponent)
						{
							continue;
						}
						const MeshResource* pMeshResource = pMeshComponent->GetMeshResource();
						if (ResourceStatus::Ready != pMeshResource->GetStatus() &&
							ResourceStatus::Optimized != pMeshResource->GetStatus())
						{
							continue;
						}

						BlendShapeComponent* pBlendShapeComponent = m_pCurrentSceneWorld->GetBlendShapeComponent(entity);
						if (pBlendShapeComponent)
						{
							continue;
						}

						// Transform
						if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
						{
							bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
						}

						// Mesh
						constexpr StringCrc programHandleIndex{ "ShadowMapProgram" };
						SubmitStaticMeshDrawCall(pMeshComponent, viewId, programHandleIndex);
					}
				}
			}
			break;
			case cd::LightType::Point:
			{
				// Initialize(if not initialized) frame buffer
				if (!lightComponent->IsShadowMapFBsValid())
				{
					for (uint16_t i = 0U; i < 6U; ++i)
					{
						/*---------bgfx cube map----------
						  0:+X 1:-X 2:+Y 3:-Y 4:+Z 5:-Z
								 +Y
							-X +Z +X -Z
								 -Y
						------------------------------------*/
						uint16_t shadowMapFB = bgfx::createFrameBuffer(lightComponent->GetShadowMapSize(),
							lightComponent->GetShadowMapSize(), bgfx::TextureFormat::R32F).idx;
						lightComponent->AddShadowMapFB(shadowMapFB);
					}
				}

				// Compute 6 light view and 1 perspective projection matrices according to ndc depth of different graphic backends
				const cd::Point lightPosition = lightComponent->GetPosition();
				float range = lightComponent->GetRange();

				cd::Matrix4x4 lightView[6] =
				{
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Direction(1.0f, 0.0f, 0.0f),	cd::Direction(0.0f, 1.0f,  0.0f)),	//Right +X
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Direction(-1.0f, 0.0f, 0.0f),	cd::Direction(0.0f, 1.0f,  0.0f)),	//Left -X
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Direction(0.0f, 1.0f, 0.0f),	cd::Direction(0.0f, 0.0f, -1.0f)),	//Top +Y
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Direction(0.0f, -1.0f, 0.0f),	cd::Direction(0.0f, 0.0f,  1.0f)),	//Bottom -Y 
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Direction(0.0f, 0.0f, 1.0f),	cd::Direction(0.0f, 1.0f,  0.0f)),	//Front +Z
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Direction(0.0f, 0.0f, -1.0f),	cd::Direction(0.0f, 1.0f,  0.0f)),	//Back -Z
				};
				cd::Matrix4x4 lightProjection = cd::Matrix4x4::Perspective(90.0f, 1.0f, 0.01f, range, ndcDepthMinusOneToOne);

				// Settings
				bgfx::setState(defaultRenderingState);
				// 6 faces
				for (uint16_t i = 0U; i < 6U; ++i)
				{
					// Settings
					uint16_t viewId = m_renderPassID[shadowNum * shadowTexturePassMaxNum + i];
					bgfx::setViewRect(viewId, 0, 0, lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize());
					bgfx::setViewFrameBuffer(viewId, static_cast<bgfx::FrameBufferHandle>(lightComponent->GetShadowMapFBs().at(i)));
					bgfx::setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xffffffff, 1.0f, 0);
					bgfx::setViewTransform(viewId, lightView[i].begin(), lightProjection.begin());

					constexpr StringCrc lightPosAndFarPlaneCrc(lightPosAndFarPlane);
					cd::Vec4f lightPosAndFarPlaneData = cd::Vec4f(lightComponent->GetPosition().x(), lightComponent->GetPosition().y(),
						lightComponent->GetPosition().z(), lightComponent->GetRange());
					GetRenderContext()->FillUniform(lightPosAndFarPlaneCrc, &lightPosAndFarPlaneData, 1);

					// Submit draw call
					for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
					{
						// No mesh attached?
						StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
						if (!pMeshComponent)
						{
							continue;
						}
						const MeshResource* pMeshResource = pMeshComponent->GetMeshResource();
						if (ResourceStatus::Ready != pMeshResource->GetStatus() &&
							ResourceStatus::Optimized != pMeshResource->GetStatus())
						{
							continue;
						}

						// Transform
						if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
						{
							bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
						}

						constexpr StringCrc programHandleIndex{ "LinearShadowMapProgram" };
						SubmitStaticMeshDrawCall(pMeshComponent, viewId, programHandleIndex);
					}
				}
			}
			break;
			case cd::LightType::Spot:
			{
				// Initialize(if not initialized) frame buffer
				if (!lightComponent->IsShadowMapFBsValid())
				{
					uint16_t shadowMapFB = bgfx::createFrameBuffer(lightComponent->GetShadowMapSize(),
						lightComponent->GetShadowMapSize(), bgfx::TextureFormat::D32F).idx;
					lightComponent->AddShadowMapFB(shadowMapFB);
				}

				// Compute 1 light view and 1 perspective projection matrices according to ndc depth of different graphic backends
				const cd::Point lightPosition = lightComponent->GetPosition();
				const cd::Point lightDirection = lightComponent->GetDirection();
				float range = lightComponent->GetRange();

				cd::Direction upOrRight = cd::Direction(0.0f, 1.0f, 0.0f) == lightDirection ? cd::Direction(0.0f, 1.0f, 0.0f) : cd::Direction(1.0f, 0.0f, 0.0f);
				cd::Matrix4x4 lightView = cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + lightDirection, upOrRight);
				cd::Matrix4x4 lightProjection = cd::Matrix4x4::Perspective(2.0f*lightComponent->GetInnerAndOuter().y(), 1.0f, 0.1f, range, ndcDepthMinusOneToOne);

				// Settings
				uint64_t state = defaultRenderingState;
				bgfx::setState(state);
				uint16_t viewId = m_renderPassID[shadowNum * shadowTexturePassMaxNum + 0];
				bgfx::setViewRect(viewId, 0, 0, lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize());
				bgfx::setViewFrameBuffer(viewId, static_cast<bgfx::FrameBufferHandle>(lightComponent->GetShadowMapFBs().at(0)));
				bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH, 0xffffffff, 1.0f, 0);
				bgfx::setViewTransform(viewId, lightView.begin(), lightProjection.begin());

				// Set transform for projecting coordinates to light space in wolrd renderer 
				lightComponent->ClearLightViewProjMatrix();
				cd::Matrix4x4 lightCSMViewProj = lightProjection * lightView;
				lightComponent->AddLightViewProjMatrix(lightCSMViewProj);

				// Submit draw call
				for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
				{
					// No mesh attached?
					StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
					if (!pMeshComponent)
					{
						continue;
					}
					const MeshResource* pMeshResource = pMeshComponent->GetMeshResource();
					if (ResourceStatus::Ready != pMeshResource->GetStatus() &&
						ResourceStatus::Optimized != pMeshResource->GetStatus())
					{
						continue;
					}

					BlendShapeComponent* pBlendShapeComponent = m_pCurrentSceneWorld->GetBlendShapeComponent(entity);
					if (pBlendShapeComponent)
					{
						continue;
					}

					// Transform
					if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
					{
						bgfx::setTransform(pTransformComponent->GetWorldMatrix().begin());
					}

					// Mesh
					constexpr StringCrc programHandleIndex{ "ShadowMapProgram" };
					SubmitStaticMeshDrawCall(pMeshComponent, viewId, programHandleIndex);
				}
			}
			break;
			}

			++shadowNum;
			if (3 == shadowNum)
			{
				break;
			}
		}
	}
}

}