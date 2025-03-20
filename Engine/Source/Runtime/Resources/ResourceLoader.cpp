#include "ResourceLoader.h"

#include <SDL_rwops.h>

#include <fstream>

namespace engine
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

std::vector<unsigned char> ResourceLoader::LoadFileFromResourceRoot(const char* pFilePath)
{
	std::vector<unsigned char> fileData;

	SDL_RWops* pRWops = SDL_RWFromFile(pFilePath, "rb");
	Sint64 dataSize = SDL_RWsize(pRWops);
	fileData.resize(dataSize);
	SDL_RWread(pRWops, fileData.data(), fileData.size());

	return fileData;
}

}