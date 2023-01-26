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

static bx::AllocatorI* GetResourceAllocator()
{
	static bx::DefaultAllocator s_allocator;
	return &s_allocator;
}

}

namespace engine
{

std::optional<MaterialComponent::TextureInfo> MaterialComponent::GetTextureInfo(cd::MaterialTextureType textureType) const
{
	auto itTextureInfo = m_textureResources.find(textureType);
	if (itTextureInfo == m_textureResources.end())
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
	m_textureTypeToBlob.clear();
	m_uberShaderOption = ShaderSchema::DefaultUberOption;
	m_textureResources.clear();
}

void MaterialComponent::Build()
{
	assert(m_pMaterialData && "Input data is not ready.");

	static int textureIndex = 0;
	uint64_t textureFlag = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	for (const auto& [textureType, textureBlob] : m_textureTypeToBlob)
	{
		std::optional<uint8_t> optTextureSlot = m_pMaterialType->GetTextureSlot(textureType);
		if (!optTextureSlot.has_value())
		{
			continue;
		}

		if (cd::MaterialTextureType::BaseColor == textureType)
		{
			textureFlag |= BGFX_TEXTURE_SRGB;
		}

		// TODO : Customized allocator and free logic.
		// It may depend on writing our own texture compile tool.
		bimg::ImageContainer* pImageContainer = bimg::imageParse(GetResourceAllocator(), textureBlob.data(), static_cast<uint32_t>(textureBlob.size()));
		const bgfx::Memory* pMemory = bgfx::makeRef(pImageContainer->m_data, pImageContainer->m_size);

		uint16_t textureWidth = pImageContainer->m_width;
		uint16_t textureHeight = pImageContainer->m_height;
		uint16_t textureDepth = pImageContainer->m_depth;
		uint16_t textureLayerCount = pImageContainer->m_numLayers;
		bgfx::TextureFormat::Enum textureFormat = static_cast<bgfx::TextureFormat::Enum>(pImageContainer->m_format);
		bool hasMips = pImageContainer->m_numMips > 1;
		
		bgfx::TextureHandle textureHandle(bgfx::kInvalidHandle);
		if (pImageContainer->m_cubeMap)
		{
			textureHandle = bgfx::createTextureCube(textureWidth, hasMips, textureLayerCount, textureFormat, textureFlag, pMemory);
		}
		else if (textureDepth > 1)
		{
			textureHandle = bgfx::createTexture3D(textureWidth, textureHeight, textureDepth, hasMips, textureFormat, textureFlag, pMemory);
		}
		else if (bgfx::isTextureValid(0, false, textureLayerCount, textureFormat, textureFlag))
		{
			textureHandle = bgfx::createTexture2D(textureWidth, textureHeight, hasMips, textureLayerCount, textureFormat, textureFlag, pMemory);
		}

		std::string samplerUniformName = "s_textureSampler";
		samplerUniformName += std::to_string(textureIndex++);
		bgfx::UniformHandle samplerHandle = bgfx::createUniform(samplerUniformName.c_str(), bgfx::UniformType::Sampler);

		TextureInfo textureInfo;
		textureInfo.slot = optTextureSlot.value();
		textureInfo.samplerHandle = samplerHandle.idx;
		textureInfo.textureHandle = textureHandle.idx;
		m_textureResources[textureType] = cd::MoveTemp(textureInfo);
	}
}

}