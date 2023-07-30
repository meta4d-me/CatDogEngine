#pragma once

#include <vector>

namespace engine
{

// ResourceLoader applies static methods to load different asset file from disk to convert to a binary block in heap memory.
class ResourceLoader final
{
public:
	ResourceLoader() = delete;
	ResourceLoader(const ResourceLoader&) = delete;
	ResourceLoader& operator=(const ResourceLoader&) = delete;
	ResourceLoader(ResourceLoader&&) = delete;
	ResourceLoader& operator=(ResourceLoader&&) = delete;
	~ResourceLoader() = delete;

	static std::vector<std::byte> LoadFile(const char* pFilePath);
	static std::vector<unsigned char> LoadFileFromResourceRoot(const char* pFilePath);
};

}