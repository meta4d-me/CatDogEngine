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

}

namespace engine
{

void DDGIComponent::SetClassificationRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    std::ifstream file(absolutePath, std::ios::binary | std::ios::in);
    if(!file.is_open())
    {
        CD_ENGINE_ERROR("Failed to open file: {0}", absolutePath);
        return;
    }

    m_classificationRawData.resize(std::filesystem::file_size(absolutePath));

    file.read(reinterpret_cast<char*>(m_classificationRawData.data()), m_classificationRawData.size());
    file.close();
}

void DDGIComponent::SetDistanceRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    std::ifstream file(absolutePath, std::ios::binary | std::ios::in);
    if(!file.is_open())
    {
        CD_ENGINE_ERROR("Failed to open file: {0}", absolutePath);
        return;
    }

    m_distanceRawData.resize(std::filesystem::file_size(absolutePath));

    file.read(reinterpret_cast<char*>(m_distanceRawData.data()), m_distanceRawData.size());
    file.close();
}

void DDGIComponent::SetIrradianceRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    std::ifstream file(absolutePath, std::ios::binary | std::ios::in);
    if(!file.is_open())
    {
        CD_ENGINE_ERROR("Failed to open file: {0}", absolutePath);
        return;
    }

    m_irradianceRawData.resize(std::filesystem::file_size(absolutePath));

    file.read(reinterpret_cast<char*>(m_irradianceRawData.data()), m_irradianceRawData.size());
    file.close();
}

void DDGIComponent::SetRelocationRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    std::ifstream file(absolutePath, std::ios::binary | std::ios::in);
    if(!file.is_open())
    {
        CD_ENGINE_ERROR("Failed to open file: {0}", absolutePath);
        return;
    }

    m_relocationRawData.resize(std::filesystem::file_size(absolutePath));

    file.read(reinterpret_cast<char*>(m_relocationRawData.data()), m_relocationRawData.size());
    file.close();
}

}
