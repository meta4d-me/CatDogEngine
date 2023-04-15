#include "ResourceLoader.h"

#include <fstream>

#include <bgfx/bgfx.h>


namespace details
{

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

namespace editor
{

std::vector<std::byte> ResourceLoader::LoadFile(const char* pFilePath)
{
	std::vector<std::byte> fileData;

	std::ifstream fin(pFilePath, std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		return fileData;
	}

	fin.seekg(0L, std::ios::end);
	size_t fileSize = fin.tellg();
	fin.seekg(0L, std::ios::beg);
	fileData.resize(fileSize);
	fin.read(reinterpret_cast<char*>(fileData.data()), fileSize);
	fin.close();

	return fileData;
}

uint16_t ResourceLoader::LoadBgfxTextureFile(const char* pFilePath)
{
	std::vector<std::byte> textureBuffer = LoadTextureFile(pFilePath);
	const bgfx::Memory* pMemory = bgfx::makeRef(textureBuffer.data(), static_cast<uint32_t>(textureBuffer.size()));
	bgfx::TextureHandle textureHandle = details::BGFXCreateTexture(512, 512, 1, false, true, 1, bgfx::TextureFormat::Enum::BC3, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP, pMemory);
	return textureHandle.idx;
}

}