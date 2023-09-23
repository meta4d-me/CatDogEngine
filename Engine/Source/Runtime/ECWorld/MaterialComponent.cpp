#include "MaterialComponent.h"

#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Scene/Material.h"
#include "Scene/Texture.h"

#include <bgfx/bgfx.h>
#include <bimg/decode.h>
#include <bx/allocator.h>

#include <cassert>
#include <filesystem>
#include <unordered_map>

namespace
{

static uint32_t textureIndex = 0;

static bx::AllocatorI* GetResourceAllocator()
{
	static bx::DefaultAllocator s_allocator;
	return &s_allocator;
}

bgfx::TextureHandle BGFXCreateTexture(
	uint16_t width, 
	uint16_t height, 
	uint16_t depth, 
	bool isCubeMap,
	bool hasMips, 
	uint16_t numLayers, 
	bgfx::TextureFormat::Enum textureFormat, 
	uint64_t textureFlags, 
	const bgfx::Memory* pMemory)
{
	bgfx::TextureHandle textureHandle = BGFX_INVALID_HANDLE;
	if (isCubeMap)
	{
		textureHandle = bgfx::createTextureCube(width, hasMips, numLayers, textureFormat, textureFlags, pMemory);
	}
	else if (depth > 1)
	{
		textureHandle = bgfx::createTexture3D(width, height, depth, hasMips, textureFormat, textureFlags, pMemory);
	}
	else if (bgfx::isTextureValid(0, false, numLayers, textureFormat, textureFlags))
	{
		textureHandle = bgfx::createTexture2D(width, height, hasMips, numLayers, textureFormat, textureFlags, pMemory);
	}
	return textureHandle;
}

uint64_t GetBGFXTextureFlag(cd::MaterialTextureType textureType, cd::TextureMapMode uMapMode, cd::TextureMapMode vMapMode)
{
	uint64_t textureFlag = 0;
	if (cd::MaterialTextureType::BaseColor == textureType ||
		cd::MaterialTextureType::Emissive == textureType)
	{
		textureFlag |= BGFX_TEXTURE_SRGB;
	}

	switch (uMapMode)
	{
	case cd::TextureMapMode::Clamp:
		textureFlag |= BGFX_SAMPLER_U_CLAMP;
		break;
	case cd::TextureMapMode::Mirror:
		textureFlag |= BGFX_SAMPLER_U_MIRROR;
		break;
	case cd::TextureMapMode::Border:
		textureFlag |= BGFX_SAMPLER_U_BORDER;
		break;
	case cd::TextureMapMode::Wrap:
	default:
		break;
	}

	switch (vMapMode)
	{
	case cd::TextureMapMode::Clamp:
		textureFlag |= BGFX_SAMPLER_V_CLAMP;
		break;
	case cd::TextureMapMode::Mirror:
		textureFlag |= BGFX_SAMPLER_V_MIRROR;
		break;
	case cd::TextureMapMode::Border:
		textureFlag |= BGFX_SAMPLER_V_BORDER;
		break;
	case cd::TextureMapMode::Wrap:
	default:
		break;
	}

	return textureFlag;
}

const std::unordered_map<engine::SkyType, engine::ShaderFeature> skyTypeToShaderFeature
{
	{ engine::SkyType::SkyBox, engine::ShaderFeature::IBL},
	{ engine::SkyType::AtmosphericScattering, engine::ShaderFeature::ATM },
};

CD_FORCEINLINE bool IsSkyTypeValid(engine::SkyType type)
{
	return skyTypeToShaderFeature.find(type) != skyTypeToShaderFeature.end();
}

}

namespace engine
{

void MaterialComponent::Init()
{
	Reset();
}

MaterialComponent::TextureInfo* MaterialComponent::GetTextureInfo(cd::MaterialTextureType textureType)
{
	auto itTextureInfo = m_textureResources.find(textureType);
	if (itTextureInfo == m_textureResources.end())
	{
		return nullptr;
	}

	return &itTextureInfo->second;
}

const MaterialComponent::TextureInfo* MaterialComponent::GetTextureInfo(cd::MaterialTextureType textureType) const
{
	auto itTextureInfo = m_textureResources.find(textureType);
	if (itTextureInfo == m_textureResources.cend())
	{
		return nullptr;
	}

	return &itTextureInfo->second;
}

void MaterialComponent::ActivateShaderFeature(engine::ShaderFeature feature)
{
	m_shaderFeatures.insert(feature);
}

void MaterialComponent::DeactiveShaderFeature(engine::ShaderFeature feature)
{
	m_shaderFeatures.erase(feature);
}

void MaterialComponent::Reset()
{
	m_pMaterialData = nullptr;
	m_pMaterialType = nullptr;
	m_shaderFeatures.clear();
	m_name.clear();
	m_albedoColor = cd::Vec3f::One();
	m_emissiveColor = cd::Vec3f::One();
	m_metallicFactor = 0.1f;
	m_roughnessFactor = 0.9f;
	m_twoSided = false;
	m_blendMode = cd::BlendMode::Opaque;
	m_alphaCutOff = 1.0f;
	m_textureResources.clear();
	m_skyType = SkyType::None;
}

const StringCrc MaterialComponent::GetFeaturesCombineCrc() const
{
	return m_pMaterialType->GetShaderSchema().GetFeaturesCombineCrc(m_shaderFeatures);
}

std::string MaterialComponent::GetFeaturesCombine() const
{
	return m_pMaterialType->GetShaderSchema().GetFeaturesCombine(m_shaderFeatures);
}

std::string MaterialComponent::GetVertexShaderName() const
{
	return std::filesystem::path(m_pMaterialType->GetShaderSchema().GetVertexShaderPath()).filename().string();
}

std::string MaterialComponent::GetFragmentShaderName() const
{
	return std::filesystem::path(m_pMaterialType->GetShaderSchema().GetFragmentShaderPath()).filename().string();
}

void MaterialComponent::AddTextureBlob(cd::MaterialTextureType textureType, cd::TextureFormat textureFormat, cd::TextureMapMode uMapMode, cd::TextureMapMode vMapMode,
	TextureBlob textureBlob, uint32_t width, uint32_t height, uint32_t depth /* = 1 */)
{
	std::optional<uint8_t> optTextureSlot = m_pMaterialType->GetTextureSlot(textureType);
	if (!optTextureSlot.has_value())
	{
		return;
	}

	TextureInfo textureInfo = m_textureResources[textureType];
	textureInfo.slot = optTextureSlot.value();
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.depth = depth;
	textureInfo.mipCount = 0;
	textureInfo.format = textureFormat;
	textureInfo.data = bgfx::makeRef(textureBlob.data(), static_cast<uint32_t>(textureBlob.size()));
	textureInfo.flag = GetBGFXTextureFlag(textureType, uMapMode, vMapMode);
	textureInfo.uvOffset = cd::Vec2f::Zero();
	textureInfo.uvScale = cd::Vec2f::One();
	m_textureResources[textureType] = cd::MoveTemp(textureInfo);
	
	// TODO : generic CPU/GPU resource manager.
	m_cacheTextureBlobs.emplace_back(cd::MoveTemp(textureBlob));
}

void MaterialComponent::AddTextureFileBlob(cd::MaterialTextureType textureType, const cd::Material* pMaterial, const cd::Texture& texture, TextureBlob textureBlob)
{
	std::optional<uint8_t> optTextureSlot = m_pMaterialType->GetTextureSlot(textureType);
	if (!optTextureSlot.has_value())
	{
		return;
	}

	bimg::ImageContainer* pImageContainer = bimg::imageParse(GetResourceAllocator(), textureBlob.data(), static_cast<uint32_t>(textureBlob.size()));
	TextureInfo textureInfo = m_textureResources[textureType];
	textureInfo.slot = optTextureSlot.value();
	textureInfo.width = pImageContainer->m_width;
	textureInfo.height = pImageContainer->m_height;
	textureInfo.depth = pImageContainer->m_depth;
	textureInfo.mipCount = pImageContainer->m_numMips;
	textureInfo.format = static_cast<cd::TextureFormat>(pImageContainer->m_format);
	textureInfo.data = bgfx::makeRef(pImageContainer->m_data, pImageContainer->m_size);
	textureInfo.flag = GetBGFXTextureFlag(textureType, texture.GetUMapMode(), texture.GetVMapMode());
	if (auto optUVScale = pMaterial->GetVec2fProperty(textureType, cd::MaterialProperty::UVScale); optUVScale.has_value())
	{
		textureInfo.uvScale = optUVScale.value();
	}
	if (auto optUVOffset = pMaterial->GetVec2fProperty(textureType, cd::MaterialProperty::UVOffset); optUVOffset.has_value())
	{
		textureInfo.uvOffset = optUVOffset.value();
	}
	m_textureResources[textureType] = cd::MoveTemp(textureInfo);
}

void MaterialComponent::Build()
{
	if (m_pMaterialData)
	{
		m_name = m_pMaterialData->GetName();
	}
	
	for (auto& [textureType, textureInfo] : m_textureResources)
	{
		textureInfo.textureHandle = BGFXCreateTexture(textureInfo.width, textureInfo.height, textureInfo.depth, false, textureInfo.mipCount > 1,
			1, static_cast<bgfx::TextureFormat::Enum>(textureInfo.format), textureInfo.flag, textureInfo.data).idx;

		std::string samplerUniformName = "s_textureSampler";
		samplerUniformName += std::to_string(textureIndex++);
		textureInfo.samplerHandle = bgfx::createUniform(samplerUniformName.c_str(), bgfx::UniformType::Sampler).idx;

		assert(textureInfo.textureHandle != bgfx::kInvalidHandle);
		assert(textureInfo.samplerHandle != bgfx::kInvalidHandle);
	}
}

void MaterialComponent::SetSkyType(SkyType crtType)
{
	// SkyType::None is a special case which is not a shader feature but means deactive ShaderFeature::IBL and ShaderFeature::ATM.

	if (SkyType::Count == crtType || m_skyType == crtType)
	{
		return;
	}

	if (IsSkyTypeValid(m_skyType))
	{
		m_shaderFeatures.erase(skyTypeToShaderFeature.at(m_skyType));
	}

	if (IsSkyTypeValid(crtType))
	{
		m_shaderFeatures.insert(skyTypeToShaderFeature.at(crtType));
	}

	m_skyType = crtType;
}

}
