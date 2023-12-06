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
// uniform name
constexpr const char* cameraPos = "u_cameraPos";
constexpr const char* lightCountAndStride = "u_lightCountAndStride";
constexpr const char* lightParams = "u_lightParams";
constexpr const char* lightPosAndFarPlane = "u_lightWorldPos_farPlane";
constexpr const char* LightDir = "u_LightDir";
constexpr const char* HeightOffsetAndshadowLength = "u_HeightOffsetAndshadowLength";

// texture name
constexpr const char* TextureShadowMap2D = "TextureShadowMap2D";
constexpr const char* TextureShadowMapCube = "TextureShadowMapCube";

//
constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;
constexpr uint64_t defaultRenderingState = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z
| BGFX_STATE_WRITE_MASK | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
constexpr uint64_t depthBufferFlags = BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL;
constexpr uint64_t linearDepthBufferFlags = BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
constexpr uint64_t clearFrameBufferFlags = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH;
}

void ShadowMapRenderer::Init()
{
	constexpr StringCrc shadowMapProgramCrc = StringCrc("ShadowMapProgram");
	GetRenderContext()->RegisterShaderProgram(shadowMapProgramCrc, { "vs_shadowMap", "fs_shadowMap" });
	constexpr StringCrc linearshadowMapProgramCrc = StringCrc("LinearShadowMapProgram");
	GetRenderContext()->RegisterShaderProgram(linearshadowMapProgramCrc, { "vs_shadowMap", "fs_shadowMap_linear" });

	for (int lightIndex = 0; lightIndex < shadowLightMaxNum; lightIndex++)
	{
		for (int mapId = 0; mapId < shadowTexturePassMaxNum; mapId++)
		{
			m_renderPassID[lightIndex * shadowTexturePassMaxNum + mapId] = GetRenderContext()->CreateView();
			bgfx::setViewName(m_renderPassID[lightIndex * shadowTexturePassMaxNum + mapId], "ShadowMapRenderer");
		}
	}
}

void ShadowMapRenderer::Warmup()
{
	GetRenderContext()->UploadShaderProgram("ShadowMapProgram");
	GetRenderContext()->UploadShaderProgram("LinearShadowMapProgram");
	GetRenderContext()->CreateUniform(lightPosAndFarPlane, bgfx::UniformType::Vec4, 1);
}

void ShadowMapRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	//UpdateViewRenderTarget(); //in Render()
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
		bool ndcDepthMinusOneToOne = cd::NDCDepth::MinusOneToOne == pMainCameraComponent->GetNDCDepth();

		auto UnProject = [&invCamViewProj](const cd::Vec4f ndcCorner)->cd::Vec3f
		{
			cd::Vec4f worldPos = invCamViewProj * ndcCorner;
			return worldPos.xyz() / worldPos.w();
		};

		uint16_t shadowNum = 0U;
		for (auto lightEntity : lightEntities)
		{
			LightComponent* lightComponent = m_pCurrentSceneWorld->GetLightComponent(lightEntity);

			// area light does not have shadowComponent
			if (false == lightComponent->GetIsCastShadow())
			{
				continue;
			}

			// render shadow map
			switch (lightComponent->GetType())
			{
			case cd::LightType::Directional:
			{
				//fbo and texture init(if not initiated) and acquire
				if (!lightComponent->IsShadowMapFBsValid())
				{
					bgfx::TextureHandle shadowMapTexture = GetRenderContext()->CreateTexture(TextureShadowMap2D, 
						lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize(), 
						1, bgfx::TextureFormat::D32F, depthBufferFlags, nullptr, 0);
					bgfx::FrameBufferHandle shadowMapFB = bgfx::createFrameBuffer(1U, &shadowMapTexture, true);
					lightComponent->AddShadowMapTexture(shadowMapTexture);
					lightComponent->AddShadowMapFB(shadowMapFB);
				}

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

				uint64_t state = defaultRenderingState;
				bgfx::setState(state);
				uint16_t viewId = m_renderPassID[shadowNum * shadowTexturePassMaxNum + 0];
				bgfx::setViewRect(viewId, 0, 0, lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize());
				bgfx::setViewFrameBuffer(viewId, lightComponent->GetShadowMapFBs().at(0));
				bgfx::setViewClear(viewId, clearFrameBufferFlags, 0x303030ff, 1.0f, 0);
				bgfx::setViewTransform(viewId, lightView.Begin(), lightProjection.Begin());

				cd::Matrix4x4 lightViewProj = lightProjection * lightView;
				lightComponent->SetLightViewProjMatrix(lightViewProj);

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
					GetRenderContext()->Submit(viewId, "ShadowMapProgram");
				}
			}
			break;
			case cd::LightType::Point:
			{
				//fbo and texture init(if not initiated) and acquire
				if (!lightComponent->IsShadowMapFBsValid())
				{
					constexpr StringCrc fbtextureName("ShadowMapTexture");

					auto fbtexture = GetRenderContext()->GetTexture(fbtextureName);
					if (!bgfx::isValid(fbtexture))				
					{
						fbtexture = bgfx::createTextureCube(lightComponent->GetShadowMapSize(),
							false, 1, bgfx::TextureFormat::R32F, depthBufferFlags);// linearDepthBufferFlags);
						GetRenderContext()->SetTexture(fbtextureName, fbtexture);
						//lightComponent->AddShadowMapTexture(fbtexture);
					}

					for (uint16_t i = 0U; i < 6; i++)
					{
						/*---------bgfx cube map----------
						  0:+X 1:-X 2:+Y 3:-Y 4:+Z 5:-Z
								 +Y
							-X +Z +X -Z
								 -Y
						------------------------------------*/
						//bgfx::Attachment fbAttachment;
						//fbAttachment.init(fbtexture, bgfx::Access::ReadWrite, i);
						bgfx::FrameBufferHandle shadowMapFB = bgfx::createFrameBuffer(1024, 1024, bgfx::TextureFormat::R32F);
						lightComponent->AddShadowMapTexture(bgfx::getTexture(shadowMapFB));
						lightComponent->AddShadowMapFB(shadowMapFB);
					}
				}

				// light view * 6 and perspective pro
				const cd::Vec3f lightPosition = lightComponent->GetPosition();
				float range = lightComponent->GetRange();

				// TODO :check direction
				cd::Matrix4x4 lightView[6] =
				{
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(1.0f, 0.0f, 0.0f),	cd::Vec3f(0.0f, 1.0f,  0.0f)),	//Right +X
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(-1.0f, 0.0f, 0.0f),	cd::Vec3f(0.0f, 1.0f,  0.0f)),	//Left -X
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(0.0f, 1.0f, 0.0f),	cd::Vec3f(0.0f, 0.0f, -1.0f)),	//Top +Y
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(0.0f, -1.0f, 0.0f),	cd::Vec3f(0.0f, 0.0f,  1.0f)),	//Bottom -Y 
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(0.0f, 0.0f, 1.0f),	cd::Vec3f(0.0f, 1.0f,  0.0f)),	//Front +Z
					cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + cd::Vec3f(0.0f, 0.0f, -1.0f),	cd::Vec3f(0.0f, 1.0f,  0.0f)),	//Back -Z
				};
				cd::Matrix4x4 lightProjection = cd::Matrix4x4::Perspective(90.0f, 1.0f, 0.01f, range, ndcDepthMinusOneToOne);

				uint64_t state = defaultRenderingState;
				bgfx::setState(state);
				for (uint16_t i = 0U; i < shadowTexturePassMaxNum; i++)
				{
					uint16_t viewId = m_renderPassID[shadowNum * shadowTexturePassMaxNum + i];
					bgfx::setViewRect(viewId, 0, 0, lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize());
					bgfx::setViewFrameBuffer(viewId, lightComponent->GetShadowMapFBs().at(i));
					bgfx::setViewClear(viewId, clearFrameBufferFlags, 0xffffffff, 1.0f, 0);
					bgfx::setViewTransform(viewId, lightView[i].Begin(), lightProjection.Begin());

					constexpr StringCrc lightPosAndFarPlaneCrc(lightPosAndFarPlane);
					cd::Vec4f lightPosAndFarPlaneData = cd::Vec4f(lightComponent->GetPosition().x(), lightComponent->GetPosition().y(),
						lightComponent->GetPosition().z(), lightComponent->GetRange());
					GetRenderContext()->FillUniform(lightPosAndFarPlaneCrc, &lightPosAndFarPlaneData, 1);

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
						GetRenderContext()->Submit(viewId, "LinearShadowMapProgram");
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
						bgfx::createTexture2D(lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize(),
							false, 1, bgfx::TextureFormat::D32F, depthBufferFlags),
					};
					bgfx::FrameBufferHandle shadowMapFB = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
					lightComponent->AddShadowMapTexture(fbtextures[0]);
					lightComponent->AddShadowMapFB(shadowMapFB);
				}

				// light view and perspective pro
				const cd::Vec3f lightPosition = lightComponent->GetPosition();
				const cd::Vec3f lightDirection = lightComponent->GetDirection();
				float range = lightComponent->GetRange();

				// TODO :check direction and determine the fov
				cd::Matrix4x4 lightView = cd::Matrix4x4::LookAt<cd::Handedness::Left>(lightPosition, lightPosition + lightDirection, cd::Vec3f(0.0f, 1.0f, 0.0f));//up or right
				cd::Matrix4x4 lightProjection = cd::Matrix4x4::Perspective(lightComponent->GetInnerAndOuter().y(), 1.0f, 0.1f, range, ndcDepthMinusOneToOne);

				uint64_t state = defaultRenderingState;
				bgfx::setState(state);
				uint16_t viewId = m_renderPassID[shadowNum * shadowTexturePassMaxNum + 0];
				bgfx::setViewRect(viewId, 0, 0, lightComponent->GetShadowMapSize(), lightComponent->GetShadowMapSize());
				bgfx::setViewFrameBuffer(viewId, lightComponent->GetShadowMapFBs().at(0));
				bgfx::setViewClear(viewId, clearFrameBufferFlags, 0x303030ff, 1.0f, 0);
				bgfx::setViewTransform(viewId, lightView.Begin(), lightProjection.Begin());

				cd::Matrix4x4 lightViewProj = lightProjection * lightView;
				lightComponent->SetLightViewProjMatrix(lightViewProj);

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
					GetRenderContext()->Submit(viewId, "ShadowMapProgram");
				}
			}
			break;
			}

			shadowNum++;
			if (3 == shadowNum)
			{
				break;
			}
		}
	}
}

}
