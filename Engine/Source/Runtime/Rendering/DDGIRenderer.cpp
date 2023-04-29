#include "DDGIRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Material/ShaderSchema.h"
#include "RenderContext.h"
#include "Rendering/DDGIDefinition.h"
#include "Scene/Texture.h"

namespace engine
{

namespace
{

constexpr const char* classificationSampler = "s_texClassification";
constexpr const char* distanceSampler = "s_texDistance";
constexpr const char* irradianceSampler = "s_texIrradiance";
constexpr const char* relocationSampler = "s_texRelocation";

constexpr uint64_t samplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP;

static constexpr uint8_t CLASSIFICATICON_GRID_SIZE = 1;
static constexpr uint8_t DISTANCE_GRID_SIZE = 14 + 2;
static constexpr uint8_t IRRADIANCE_GRID_SIZE = 6 + 2;
static constexpr uint8_t RELOCATION_GRID_SIZE = 1;

void CreatDDGITexture(DDGITextureType type, DDGIComponent* pDDGIComponent, RenderContext* pRenderContext)
{
	assert(pDDGIComponent);
	assert(pRenderContext);

	const char* pName = GetDDGITextureTypeName(type);
	cd::Vec3f probNum = pDDGIComponent->GetDimension() / pDDGIComponent->GetSpacing();

	cd::Vec2f textureSize = cd::Vec2f(0.0f, 0.0f);
	bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Enum::Unknown;
	const void* data = nullptr;
	uint32_t dataSize = 0;
	switch(type)
	{
		case DDGITextureType::Classification:
			textureSize = cd::Vec2f(probNum.y() * probNum.z(), probNum.x()) * CLASSIFICATICON_GRID_SIZE;
			format = bgfx::TextureFormat::Enum::R32F;
			data = reinterpret_cast<const void *>(pDDGIComponent->GetClassificationRawData());
			dataSize = pDDGIComponent->GetClassificationSize();
			break;
		case DDGITextureType::Distance:
			textureSize = cd::Vec2f(probNum.y() * probNum.z(), probNum.x()) * DISTANCE_GRID_SIZE;
			format = bgfx::TextureFormat::Enum::RG32F;
			data = reinterpret_cast<const void *>(pDDGIComponent->GetDistanceRawData());
			dataSize = pDDGIComponent->GetDistanceSize();
			break;
		case DDGITextureType::Irradiance:
			textureSize = cd::Vec2f(probNum.y() * probNum.z(), probNum.x()) * IRRADIANCE_GRID_SIZE;
			// TODO : RGBA16U
			format = bgfx::TextureFormat::Enum::RGBA16F;
			data = reinterpret_cast<const void *>(pDDGIComponent->GetIrradianceRawData());
			dataSize = pDDGIComponent->GetIrradianceSize();
			break;
		case DDGITextureType::Relocation:
			textureSize = cd::Vec2f(probNum.y() * probNum.z(), probNum.x()) * RELOCATION_GRID_SIZE;
			format = bgfx::TextureFormat::Enum::RGBA16F;
			data = reinterpret_cast<const void *>(pDDGIComponent->GetRelocationRawData());
			dataSize = pDDGIComponent->GetRelocationSize();
			break;
	}

	if(nullptr != data && dataSize > 0)
	{
		pRenderContext->CreateTexture(pName, static_cast<uint16_t>(textureSize.x()), static_cast<uint16_t>(textureSize.y()), 1, format, samplerFlags, data, dataSize);
	}
	else
	{
		CD_ENGINE_WARN("DDGIRenderer faild to create texture {0}!", pName);
	}
}

}

void DDGIRenderer::Init()
{
	assert(m_pCurrentSceneWorld && "Unknown Scene World pointer!");
	m_pDDGIComponent = m_pCurrentSceneWorld->GetDDGIComponent(m_pCurrentSceneWorld->GetDDGIEntity());
	assert(m_pDDGIComponent && "Unknown DDGI Component pointer!");

	bgfx::setViewName(GetViewID(), "WorldRenderer");

	m_pRenderContext->CreateUniform(classificationSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform(distanceSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform(irradianceSampler, bgfx::UniformType::Sampler);
	m_pRenderContext->CreateUniform(relocationSampler, bgfx::UniformType::Sampler);

	// Temporary code.
	m_pDDGIComponent->SetAmbientMultiplier(1.0);
	m_pDDGIComponent->SetDimension(cd::Vec3f(4.0f, 5.0f, 2.0f));
	m_pDDGIComponent->SetNormalBias(0.0f);
	m_pDDGIComponent->SetSpacing(cd::Vec3f(1.0f, 1.0f, 1.0f));
	m_pDDGIComponent->SetViewBias(0.0f);

	m_pDDGIComponent->SetClassificationRawData("ddgi/classification.bin");
	m_pDDGIComponent->SetDistanceRawData("ddgi/distance.bin");
	m_pDDGIComponent->SetIrradianceRawData("ddgi/irradiance.bin");
	m_pDDGIComponent->SetRelocationRawData("ddgi/relocation.bin");

	CreatDDGITexture(DDGITextureType::Classification, m_pDDGIComponent, m_pRenderContext);
	CreatDDGITexture(DDGITextureType::Distance, m_pDDGIComponent, m_pRenderContext);
	CreatDDGITexture(DDGITextureType::Irradiance, m_pDDGIComponent, m_pRenderContext);
	CreatDDGITexture(DDGITextureType::Relocation, m_pDDGIComponent, m_pRenderContext);
}

void DDGIRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	bgfx::setViewFrameBuffer(GetViewID(),*GetRenderTarget()->GetFrameBufferHandle());
	bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void DDGIRenderer::Render(float deltaTime)
{
	for(Entity entity : m_pCurrentSceneWorld->GetMaterialEntities())
	{
		MaterialComponent* pMaterialComponent = m_pCurrentSceneWorld->GetMaterialComponent(entity);
		if(!pMaterialComponent || pMaterialComponent->GetMaterialType() != m_pCurrentSceneWorld->GetDDGIMaterialType())
		{
			continue;
		}

		// No mesh attached
		StaticMeshComponent* pMeshComponent = m_pCurrentSceneWorld->GetStaticMeshComponent(entity);
		if(!pMeshComponent)
		{
			continue;
		}

		// Transform
		if(TransformComponent* pTransformComponent = m_pCurrentSceneWorld->GetTransformComponent(entity))
		{
			bgfx::setTransform(pTransformComponent->GetWorldMatrix().Begin());
		}

		// Mesh
		bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle(pMeshComponent->GetVertexBuffer()));
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle(pMeshComponent->GetIndexBuffer()));

		for(const auto& [textureType, textureInfo] : pMaterialComponent->GetTextureResources())
		{
			std::optional<MaterialComponent::TextureInfo> optTextureInfo = pMaterialComponent->GetTextureInfo(textureType);
			if(optTextureInfo.has_value())
			{
				const MaterialComponent::TextureInfo& textureInfo = optTextureInfo.value();
				bgfx::setTexture(textureInfo.slot, bgfx::UniformHandle(textureInfo.samplerHandle), bgfx::TextureHandle(textureInfo.textureHandle));
			}
		}

		bgfx::setTexture(1, m_pRenderContext->GetUniform(StringCrc(classificationSampler)),
						 m_pRenderContext->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Classification))));
		bgfx::setTexture(2, m_pRenderContext->GetUniform(StringCrc(distanceSampler)),
						 m_pRenderContext->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Distance))));
		bgfx::setTexture(3, m_pRenderContext->GetUniform(StringCrc(irradianceSampler)),
						 m_pRenderContext->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Irradiance))));
		bgfx::setTexture(4, m_pRenderContext->GetUniform(StringCrc(relocationSampler)),
						 m_pRenderContext->GetTexture(StringCrc(GetDDGITextureTypeName(DDGITextureType::Relocation))));

		constexpr uint64_t state = BGFX_STATE_WRITE_MASK | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
		bgfx::setState(state);

		bgfx::submit(GetViewID(), bgfx::ProgramHandle(pMaterialComponent->GetShadingProgram()));
	}
}

void DDGIRenderer::UpdateClassificationTexture(const char* path)
{
	// m_pRenderContext->Destory(StringCrc(m_pDDGIComponent->GetClassificationTexturePath().c_str()));
	// m_pRenderContext->CreateTexture(path, samplerFlags);
	// m_pDDGIComponent->SetClassificationTexturePath(path);
}

void DDGIRenderer::UpdateDistanceTexture(const char* path)
{
	// m_pRenderContext->Destory(StringCrc(m_pDDGIComponent->GetDistanceTexturePath().c_str()));
	// m_pRenderContext->CreateTexture(path, samplerFlags);
	// m_pDDGIComponent->SetDistanceTexturePath(path);
}

void DDGIRenderer::UpdateIrradianceTexture(const char* path)
{
	// m_pRenderContext->Destory(StringCrc(m_pDDGIComponent->GetIrradianceTexturePath().c_str()));
	// m_pRenderContext->CreateTexture(path, samplerFlags);
	// m_pDDGIComponent->SetIrradianceTexturePath(path);
}

void DDGIRenderer::UpdateRelocationTexture(const char* path)
{
	// m_pRenderContext->Destory(StringCrc(m_pDDGIComponent->GetRelocationTexturePath().c_str()));
	// m_pRenderContext->CreateTexture(path, samplerFlags);
	// m_pDDGIComponent->SetRelocationTexturePath(path);
}

}
