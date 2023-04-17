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

}

namespace engine
{

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
	m_textureTypeToFileBlob.clear();
	m_uberShaderOption = ShaderSchema::DefaultUberOption;
	m_textureResources.clear();
}

void MaterialComponent::AddTextureBlob(cd::MaterialTextureType textureType, cd::TextureFormat textureFormat, TextureBlob textureBlob, uint32_t width, uint32_t height, uint32_t depth /* = 1 */)
{
	std::optional<uint8_t> optTextureSlot = m_pMaterialType->GetTextureSlot(textureType);
	if (!optTextureSlot.has_value())
	{
		return;
	}

	uint64_t textureFlag = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	if (cd::MaterialTextureType::BaseColor == textureType)
	{
		textureFlag |= BGFX_TEXTURE_SRGB;
	}

	const bgfx::Memory* pMemory = bgfx::copy(textureBlob.data(), static_cast<uint32_t>(textureBlob.size()));

	m_textureResources[textureType] = TextureInfo();
	TextureInfo& textureInfo = m_textureResources[textureType];
	textureInfo.slot = optTextureSlot.value();
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.depth = depth;
	textureInfo.format = textureFormat;
	textureInfo.textureHandle =
		BGFXCreateTexture(
			textureInfo.width,
			textureInfo.height,
			textureInfo.depth,
			false,
			textureInfo.mipCount > 1,
			1,
			static_cast<bgfx::TextureFormat::Enum>(textureInfo.format),
			textureFlag,
			pMemory
		).idx;

	std::string samplerUniformName = "s_textureSampler";
	samplerUniformName += std::to_string(textureIndex++);
	textureInfo.samplerHandle = bgfx::createUniform(samplerUniformName.c_str(), bgfx::UniformType::Sampler).idx;
}

void MaterialComponent::Build()
{
	// Load texture files
	for (const auto& [textureType, textureFileBlob] : m_textureTypeToFileBlob)
	{
		std::optional<uint8_t> optTextureSlot = m_pMaterialType->GetTextureSlot(textureType);
		if (!optTextureSlot.has_value())
		{
			continue;
		}

		uint64_t textureFlag = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
		if (cd::MaterialTextureType::BaseColor == textureType)
		{
			textureFlag |= BGFX_TEXTURE_SRGB;
		}

		// TODO : Customized allocator and free logic.
		// It may depend on writing our own texture compile tool.
		bimg::ImageContainer* pImageContainer = bimg::imageParse(GetResourceAllocator(), textureFileBlob.data(), static_cast<uint32_t>(textureFileBlob.size()));
		const bgfx::Memory* pMemory = bgfx::makeRef(pImageContainer->m_data, pImageContainer->m_size);

		m_textureResources[textureType] = TextureInfo();
		TextureInfo& textureInfo = m_textureResources[textureType];
		textureInfo.slot = optTextureSlot.value();
		textureInfo.samplerHandle = bgfx::kInvalidHandle;
		textureInfo.textureHandle = bgfx::kInvalidHandle;
		textureInfo.width = pImageContainer->m_width;
		textureInfo.height = pImageContainer->m_height;
		textureInfo.depth = pImageContainer->m_depth;
		textureInfo.mipCount = pImageContainer->m_numMips;
		textureInfo.format = static_cast<cd::TextureFormat>(pImageContainer->m_format);
		textureInfo.textureHandle =
			BGFXCreateTexture(
				textureInfo.width,
				textureInfo.height,
				textureInfo.depth,
				pImageContainer->m_cubeMap, 
				textureInfo.mipCount > 1,
				pImageContainer->m_numLayers,
				static_cast<bgfx::TextureFormat::Enum>(textureInfo.format),
				textureFlag, 
				pMemory
			).idx;

		std::string samplerUniformName = "s_textureSampler";
		samplerUniformName += std::to_string(textureIndex++);
		textureInfo.samplerHandle = bgfx::createUniform(samplerUniformName.c_str(), bgfx::UniformType::Sampler).idx;
		assert(textureInfo.textureHandle != bgfx::kInvalidHandle);
		assert(textureInfo.samplerHandle != bgfx::kInvalidHandle);
	}
}

}