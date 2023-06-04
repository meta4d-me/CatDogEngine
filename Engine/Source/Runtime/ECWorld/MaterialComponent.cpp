#include "MaterialComponent.h"

#include "Material/MaterialType.h"
#include "Scene/Material.h"
#include "Scene/Texture.h"

#include <bgfx/bgfx.h>
#include <bimg/decode.h>
#include <bx/allocator.h>

#include <cassert>
#include <filesystem>

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
	bgfx::TextureHandle textureHandle(bgfx::kInvalidHandle);
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

}

namespace engine
{

void MaterialComponent::Init(const engine::MaterialType* pMaterialType, const cd::Material* pMaterialData)
{
	Reset();

	m_pMaterialData = pMaterialData;
	m_pMaterialType = pMaterialType;
}

std::optional<const MaterialComponent::TextureInfo> MaterialComponent::GetTextureInfo(cd::MaterialTextureType textureType) const
{
	auto itTextureInfo = m_textureResources.find(textureType);
	if (itTextureInfo == m_textureResources.cend())
	{
		return std::nullopt;
	}

	return itTextureInfo->second;
}

void MaterialComponent::SetUberShaderOption(StringCrc uberOption)
{
	m_uberShaderOption = uberOption;
}

StringCrc MaterialComponent::GetUberShaderOption() const
{
	return m_uberShaderOption;
}

uint16_t MaterialComponent::GetShadingProgram() const
{
	return m_pMaterialType->GetShaderSchema().GetCompiledProgram(m_uberShaderOption);
}

void MaterialComponent::Reset()
{
	m_pMaterialData = nullptr;
	m_pMaterialType = nullptr;
	m_uberShaderOption = ShaderSchema::DefaultUberOption;
	m_albedoColor = cd::Vec3f::One();
	m_emissiveColor = cd::Vec3f::One();
	m_textureResources.clear();
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
}

void MaterialComponent::AddTextureFileBlob(cd::MaterialTextureType textureType, const cd::Texture& texture, TextureBlob textureBlob)
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
	textureInfo.uvOffset = texture.GetUVOffset();
	textureInfo.uvScale = texture.GetUVScale();
	m_textureResources[textureType] = cd::MoveTemp(textureInfo);
}

void MaterialComponent::Build()
{
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

}