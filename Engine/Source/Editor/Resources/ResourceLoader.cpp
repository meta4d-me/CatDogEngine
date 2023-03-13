#include "ResourceLoader.h"

#include <fstream>

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

std::vector<std::byte> ResourceLoader::LoadTextureFile(const char* pFilePath)
{
	return LoadFile(pFilePath);
}

std::vector<std::byte> ResourceLoader::LoadShader(const char* pFilePath)
{
	return LoadFile(pFilePath);
}

}