#include "DDGIComponent.h"

#include "Log/Log.h"

#include <filesystem>
//#include <format>
#include <fstream>
#include <iostream>

namespace
{

CD_FORCEINLINE std::string GetBinaryFileRealPath(const std::string &path)
{
    //return std::format("{}Textures/{}", CDPROJECT_RESOURCES_ROOT_PATH, path);
    std::string filePath = CDPROJECT_RESOURCES_ROOT_PATH;
    filePath += "Textures/" + path;
    return filePath;
}

void ReadTextureBinaryFile(const std::string& path, std::vector<uint8_t>& buffer)
{
    std::ifstream file(path, std::ios::binary | std::ios::in);
    if(!file.is_open())
    {
        CD_ENGINE_ERROR("Failed to open file: {0}", path);
        return;
    }

    buffer.resize(std::filesystem::file_size(path));
    file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
    file.close();
}

}

namespace engine
{

void DDGIComponent::SetClassificationRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_classificationRawData);
}

void DDGIComponent::SetDistanceRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_distanceRawData);
}

void DDGIComponent::SetIrradianceRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_irradianceRawData);
}

void DDGIComponent::SetRelocationRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_relocationRawData);
}

}
