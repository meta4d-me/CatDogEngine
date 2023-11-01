#include "ShadowMapRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "ECWorld/LightComponent.h"
#include "LightUniforms.h"
#include "Material/ShaderSchema.h"
#include "Math/Transform.hpp"
#include "RenderContext.h"

#include <string>

namespace engine
{

namespace
{
constexpr const char* cameraPos = "u_cameraPos";

constexpr const char* lightCountAndStride = "u_lightCountAndStride";
constexpr const char* lightParams = "u_lightParams";

constexpr const char* LightDir = "u_LightDir";
constexpr const char* HeightOffsetAndshadowLength = "u_HeightOffsetAndshadowLength";

constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr uint64_t defaultRenderingState = BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;

}

void ShadowMapRenderer::Init()
{
	GetRenderContext()->CreateProgram("ShadowMapProgram", "vs_shadowMap.bin", "fs_shadowMap.bin");
	GetRenderContext()->CreateUniform(cameraPos, bgfx::UniformType::Vec4, 1);

	GetRenderContext()->CreateUniform(lightCountAndStride, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(lightParams, bgfx::UniformType::Vec4, LightUniform::VEC4_COUNT);

	GetRenderContext()->CreateUniform(LightDir, bgfx::UniformType::Vec4, 1);
	GetRenderContext()->CreateUniform(HeightOffsetAndshadowLength, bgfx::UniformType::Vec4, 1);

	for (int i = 0; i < 6; i++)
	{
		m_renderPassID[i] = GetRenderContext()->CreateView();
		bgfx::setViewName(m_renderPassID[i], "ShadowMapRenderer");
	}
}

void ShadowMapRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	//UpdateViewRenderTarget();
}

void ShadowMapRenderer::Render(float deltaTime)
{
	// TODO : Remove it. If every renderer need to submit camera related uniform, it should be done not inside Renderer class.
	const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();

	// Submit uniform values : light settings
	auto lightEntities = m_pCurrentSceneWorld->GetLightEntities();

	if (!lightEntities.empty())
	{
		//camera 
		CameraComponent* pMainCameraComponent = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity());
		const cd::Matrix4x4 camView = pMainCameraComponent->GetViewMatrix();
		const cd::Matrix4x4 camProj = pMainCameraComponent->GetProjectionMatrix();
		const cd::Matrix4x4 invCamViewProj = (camProj * camView).Inverse();

		auto UnProject = [&invCamViewProj](const cd::Vec4f ndcCorner)->cd::Vec3f
		{
			cd::Vec4f worldPos = invCamViewProj * ndcCorner;
			return worldPos.xyz() / worldPos.w();
		};

		for (auto lightEntity : lightEntities)
		{
			LightComponent* lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntity);

			switch (lightComponent->GetType())
			{
			case cd::LightType::Directional:
			{
				//fbo and texture init(if not initiated) and acquire
				if (!lightComponent->IsShadowMapFBsValid())
				{
					bgfx::TextureHandle fbtextures[] =
					{
						bgfx::createTexture2D(
							   lightComponent->GetShadowMapSize()
							, lightComponent->GetShadowMapSize()
							, false
							, 1
							, bgfx::TextureFormat::D16
							, BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL
							),
					};
					bgfx::FrameBufferHandle shadowMapFB = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
					//lightComponent->AddShadowMapTexture(fbtextures[0]);
					lightComponent->AddShadowMapFB(shadowMapFB);
				}

				bool ndcDepthMinusOneToOne = cd::NDCDepth::MinusOneToOne == pMainCameraComponent->GetNDCDepth();
				// light view and orthographic pro

				std::vector<cd::Vec3f> frustumCorners;
				if (ndcDepthMinusOneToOne)
				{
					const std::vector<cd::Vec3f> temp = {
						UnProject(cd::Vec4f(-1, -1,  1, 1)),	// near
						UnProject(cd::Vec4f(-1, -1, -1, 1)),	// far
						UnProject(cd::Vec4f(-1,  1,  1, 1)),	// near
						UnProject(cd::Vec4f(-1,  1, -1, 1)),	// far
						UnProject(cd::Vec4f(1, -1,  1, 1)),	// near
						UnProject(cd::Vec4f(1, -1, -1, 1)),	// far
						UnProject(cd::Vec4f(1,  1,  1, 1)),	// near
						UnProject(cd::Vec4f(1,  1, -1, 1))	// far
					};
					frustumCorners = std::move(temp);
				}
				else
				{
					const std::vector<cd::Vec3f> temp = {	
						UnProject(cd::Vec4f(-1, -1, 1, 1)),	// near
						UnProject(cd::Vec4f(-1, -1, 0, 1)),	// far
						UnProject(cd::Vec4f(-1,  1, 1, 1)),	// near
						UnProject(cd::Vec4f(-1,  1, 0, 1)),	// far
						UnProject(cd::Vec4f(1, -1, 1, 1)),	// near
						UnProject(cd::Vec4f(1, -1, 0, 1)),	// far
						UnProject(cd::Vec4f(1,  1, 1, 1)),		// near
						UnProject(cd::Vec4f(1,  1, 0, 1))		// far
					};
					frustumCorners = std::move(temp);
				}

				cd::Vec3f center = cd::Vec3f(0, 0, 0);
				for (const auto& corner : frustumCorners)
				{
					center += corner;
				}
				center /= static_cast<float>(frustumCorners.size());

				cd::Vec3f lightDirection = lightComponent->GetDirection();
				cd::Matrix4x4 lightView = cd::Matrix4x4::LookAt<cd::Handedness::Left>(center, center + lightDirection, cd::Vec3f(0.0f, 1.0f, 0.0f));

				float minX = std::numeric_limits<float>::max();
				float maxX = std::numeric_limits<float>::lowest();
				float minY = std::numeric_limits<float>::max();
				float maxY = std::numeric_limits<float>::lowest();
				float minZ = std::numeric_limits<float>::max();
				float maxZ = std::numeric_limits<float>::lowest();
				for (const auto& corner : frustumCorners)
				{
					const auto lightSpaceCorner = lightView * cd::Vec4f(corner.x(), corner.y(), corner.z(), 1.0);
					minX = std::min(minX, lightSpaceCorner.x());
					maxX = std::max(maxX, lightSpaceCorner.x());
					minY = std::min(minY, lightSpaceCorner.y());
					maxY = std::max(maxY, lightSpaceCorner.y());
					minZ = std::min(minZ, lightSpaceCorner.z());
					maxZ = std::max(maxZ, lightSpaceCorner.z());
				}

				cd::Matrix4x4 lightProjection = cd::Matrix4x4::Orthographic(minX, maxX, maxY, minY, minZ, maxZ, 0, ndcDepthMinusOneToOne);
				//cd::Matrix4x4 lightProjection = cd::Matrix4x4::Orthographic(-200, 200, 200, -200, -200, 200, 0, ndcDepthMinusOneToOne);

				uint64_t state = defaultRenderingState;

				bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

				for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
				{
					// No mesh attached?
					StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
					if (!pMeshComponent)
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
						bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
					}

					// Mesh
					UpdateStaticMeshComponent(pMeshComponent);

					bgfx::setState(state);

					bgfx::setViewTransform(m_renderPassID[0], lightView.Begin(), lightProjection.Begin());
					bgfx::setViewRect(m_renderPassID[0], 0, 0, lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize());
					bgfx::setViewFrameBuffer(m_renderPassID[0], lightComponent->GetShadowMapFBs().at(0));

					constexpr StringCrc shadowMapProgram("ShadowMapProgram");
					bgfx::submit(m_renderPassID[0], GetRenderContext()->GetProgram(shadowMapProgram));
				}
			}
			break;
			case cd::LightType::Point:
			{
				//fbo and texture init(if not initiated) and acquire
				if (!lightComponent->IsShadowMapFBsValid())
				{
					for (uint16_t i = 0U; i < 6; i++)
					{
						bgfx::TextureHandle fbtextures[] =
						{
							bgfx::createTexture2D(
								   lightComponent->GetShadowMapSize()
								, lightComponent->GetShadowMapSize()
								, false
								, 1
								, bgfx::TextureFormat::D16
								, BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL
								),
						};
						bgfx::FrameBufferHandle shadowMapFB = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
						//lightComponent->AddShadowMapTexture(fbtextures[0]);
						lightComponent->AddShadowMapFB(shadowMapFB);
					}
				}

				bool ndcDepthMinusOneToOne = cd::NDCDepth::MinusOneToOne == pMainCameraComponent->GetNDCDepth();
				// light view * 6 and perspective pro
				const cd::Vec3f lightPosition = lightComponent->GetPosition();
				float range = lightComponent->GetRange();

				// TODO :check direction
				cd::Matrix4x4 lightView[6] = 
				{
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(-1.0f, 0.0f, 0.0f),	cd::Vec3f(0.0f, 1.0f, 0.0f)),	//Left
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(1.0f, 0.0f, 0.0f),	cd::Vec3f(0.0f, 1.0f, 0.0f)),	//Right
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(0.0f, -1.0f, 0.0f),	cd::Vec3f(0.0f, 0.0f, 1.0f)),	//Bottom
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(0.0f, 1.0f, 0.0f),	cd::Vec3f(0.0f, 0.0f, 1.0f)),	//Top
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(0.0f, 0.0f, -1.0f),	cd::Vec3f(0.0f, 1.0f, 0.0f)),	//Front
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(0.0f, 0.0f, 1.0f),	cd::Vec3f(0.0f, 1.0f, 0.0f)),	//Back
				};
				cd::Matrix4x4 lightProjection = cd::Matrix4x4::Perspective(90.0f, 1.0f, 1.0f, range, ndcDepthMinusOneToOne);

				uint64_t state = defaultRenderingState;	
				bgfx::setState(state);
				for (uint16_t i = 0U; i < 6; i++)
				{
					bgfx::setViewClear(m_renderPassID[i], BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
					bgfx::setViewRect(m_renderPassID[i], 0, 0, lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize());
					bgfx::setViewFrameBuffer(m_renderPassID[i], lightComponent->GetShadowMapFBs().at(i));
					bgfx::setViewTransform(m_renderPassID[i], lightView[i].Begin(), lightProjection.Begin());
					for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
					{
						// No mesh attached?
						StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
						if (!pMeshComponent)
						{
							continue;
						}

						// Transform
						if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
						{
							bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
						}
						UpdateStaticMeshComponent(pMeshComponent);

						constexpr StringCrc shadowMapProgram("ShadowMapProgram");
						bgfx::submit(m_renderPassID[i], GetRenderContext()->GetProgram(shadowMapProgram));
					}
				}
			}
			break;
			case cd::LightType::Spot:
			{
				//fbo and texture init(if not initiated) and acquire
				if (!lightComponent->IsShadowMapFBsValid())
				{
					bgfx::TextureHandle fbtextures[] =
					{
						bgfx::createTexture2D(
							   lightComponent->GetShadowMapSize()
							, lightComponent->GetShadowMapSize()
							, false
							, 1
							, bgfx::TextureFormat::D16
							, BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL
							),
					};
					bgfx::FrameBufferHandle shadowMapFB = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
					//lightComponent->AddShadowMapTexture(fbtextures[0]);
					lightComponent->AddShadowMapFB(shadowMapFB);
				}

				bool ndcDepthMinusOneToOne = cd::NDCDepth::MinusOneToOne == pMainCameraComponent->GetNDCDepth();
				// light view and perspective pro
				const cd::Vec3f lightPosition = lightComponent->GetPosition();
				const cd::Vec3f lightDirection = lightComponent->GetDirection();
				float range = lightComponent->GetRange();

				// TODO :check direction and determine the fov
				cd::Matrix4x4 lightView = cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + lightDirection, cd::Vec3f(0.0f, 1.0f, 0.0f));//up or right
				cd::Matrix4x4 lightProjection = cd::Matrix4x4::Perspective(lightComponent->GetInnerAndOuter().y(), 1.0f, 0.01f, range, ndcDepthMinusOneToOne);

				uint64_t state = defaultRenderingState;
				bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
				bgfx::setState(state);


				for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
				{
					// No mesh attached?
					StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
					if (!pMeshComponent)
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
						bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
					}
					// Mesh 
					
					UpdateStaticMeshComponent(pMeshComponent);
					
					bgfx::setViewTransform(GetViewID(), lightView.Begin(), lightProjection.Begin());
					bgfx::setViewRect(GetViewID(), 0, 0, lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize());
					bgfx::setViewFrameBuffer(GetViewID(), lightComponent->GetShadowMapFBs()[0]);
					constexpr StringCrc shadowMapProgram("ShadowMapProgram");
					bgfx::submit(GetViewID(), GetRenderContext()->GetProgram(shadowMapProgram));
				}
			}
			break;
			case cd::LightType::Rectangle:
			break;
			}
		}
		/*
			for (Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
			{
				// No mesh attached?
				StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
				if (!pMeshComponent)
				{
					continue;
				}

				BlendShapeComponent* pBlendShapeComponent = m_pCurrentSceneWorld->GetBlendShapeComponent(entity);
				if (pBlendShapeComponent)
				{
					continue;
				}

				// SkinMesh
				if (m_pCurrentSceneWorld->GetAnimationComponent(entity))
				{
					continue;
				}

				// Transform
				if (TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
				{
					bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
				}

				// Mesh
				UpdateStaticMeshComponent(pMeshComponent);

				// Submit uniform values : camera settings
				constexpr StringCrc cameraPosCrc(cameraPos);
				GetRenderContext()->FillUniform(cameraPosCrc, &cameraTransform.GetTranslation().x(), 1);


			}
		*/
	}

}

}
