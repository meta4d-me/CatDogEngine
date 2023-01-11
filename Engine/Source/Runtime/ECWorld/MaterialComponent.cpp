#include "MaterialComponent.h"

#include "Material/MaterialType.h"
#include "Scene/Material.h"
#include "Scene/Texture.h"

#include <bgfx/bgfx.h>

#include <cassert>
#include <filesystem>

namespace engine
{

void MaterialComponent::SetTextureInfo(cd::MaterialTextureType textureType, uint8_t slot, uint16_t samplerHandle, uint16_t textureHandle)
{
	assert(!m_textureResources.contains(textureType));
	m_textureResources[textureType] = { .slot = slot, .samplerHandle = samplerHandle, .textureHandle = textureHandle };
}

std::optional<MaterialComponent::TextureInfo> MaterialComponent::GetTextureInfo(cd::MaterialTextureType textureType) const
{
	auto itTextureInfo = m_textureResources.find(textureType);
	if (itTextureInfo == m_textureResources.end())
	{
		return std::nullopt;
	}

	return itTextureInfo->second;
}

void MaterialComponent::BuildTexture(const char* pTextureFilePath)
{
	std::filesystem::path inputFilePath = pTextureFilePath;
	std::string outputFilePath = inputFilePath.stem().generic_string().c_str();
	outputFilePath += ".dds";

	//constexpr uint64_t textureSamplerFlags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	//std::optional<cd::TextureID> optBaseColorMap = assignedMaterial.GetTextureID(cd::MaterialTextureType::BaseColor);
	//if (optBaseColorMap.has_value())
	//{
	//	const cd::Texture& baseColorMap = pSceneDatabase->GetTexture(optBaseColorMap.value().Data());
	//	std::filesystem::path hackFilePath = baseColorMap.GetPath();
	//	std::string ddsFilePath = hackFilePath.stem().generic_string().c_str();
	//	ddsFilePath += ".dds";
	//	bgfx::TextureHandle textureHandle = m_pRenderContext->CreateTexture(ddsFilePath.c_str(), textureSamplerFlags | BGFX_TEXTURE_SRGB);
	//	if (bgfx::isValid(textureHandle))
	//	{
	//		bgfx::UniformHandle samplerHandle = m_pRenderContext->CreateUniform(std::format("s_textureBaseColor{}", mesh.GetMaterialID().Data()).c_str(), bgfx::UniformType::Sampler);
	//		materialComponent.SetTextureInfo(cd::MaterialTextureType::BaseColor, 0, samplerHandle.idx, textureHandle.idx);
	//	}
	//	else
	//	{
	//		isMissingTexture = true;
	//	}
	//}
}

void MaterialComponent::BuildShader(const char* pVSFilePath, const char* pFSFilePath)
{
	//std::string shaderFileFullPath = std::format("{}Shaders/{}", CDENGINE_RESOURCES_ROOT_PATH, pFilePath);
	//std::ifstream fin(shaderFileFullPath, std::ios::in | std::ios::binary);
	//if (!fin.is_open())
	//{
	//	return bgfx::ShaderHandle(bgfx::kInvalidHandle);
	//}
	//
	//fin.seekg(0L, std::ios::end);
	//size_t fileSize = fin.tellg();
	//fin.seekg(0L, std::ios::beg);
	//uint8_t* pRawData = new uint8_t[fileSize];
	//fin.read(reinterpret_cast<char*>(pRawData), fileSize);
	//fin.close();
	//
	//const bgfx::Memory* pMemory = bgfx::makeRef(pRawData, static_cast<uint32_t>(fileSize));
	//bgfx::ShaderHandle handle = bgfx::createShader(pMemory);
	//m_shadingProgram = bgfx::createProgram(vsh, fsh); m_pRenderContext->CreateProgram("MissingTextures", m_pStandardMaterialType->GetVertexShaderName(), "fs_missing_textures.bin");
}

void MaterialComponent::Build()
{
	assert(m_pMaterialData && "Input data is not ready.");

	for (const cd::Texture* pTextureData : m_pTextureDatas)
	{
		BuildTexture(pTextureData->GetPath());
	}
	//BuildShader(m_pMaterialType->GetVertexShaderName(), missRequiredTextures ? "fs_missing_textures.bin" : m_pMaterialType->GetFragmentShaderName());
}

}