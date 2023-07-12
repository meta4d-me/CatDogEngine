#include "DDGIComponent.h"

#include "Log/Log.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

namespace
{

CD_FORCEINLINE std::string GetBinaryFileRealPath(const std::string &path)
{
    return std::format("{}Textures/{}", CDPROJECT_RESOURCES_ROOT_PATH, path);
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

void DDGIComponent::SetClassificationRawData(const std::shared_ptr<std::vector<uint8_t>>& classification)
{
    m_classificationRawData = std::move(*classification);
}

void DDGIComponent::SetDistanceRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_distanceRawData);
}
void DDGIComponent::SetDistanceRawData(const std::shared_ptr<std::vector<uint8_t>>& irrdiance)
{
    m_irradianceRawData = std::move(*irrdiance);
}

void DDGIComponent::SetIrradianceRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_irradianceRawData);
}

void DDGIComponent::SetIrradianceRawData(const std::shared_ptr<std::vector<uint8_t>>& irrdiance)
{
    m_irradianceRawData = std::move(*irrdiance);
}

void DDGIComponent::SetRelocationRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_relocationRawData);
}

void DDGIComponent::SetRelocationRawData(const std::shared_ptr<std::vector<uint8_t>>& relocation)
{
    m_relocationRawData = std::move(*relocation);
}

}
