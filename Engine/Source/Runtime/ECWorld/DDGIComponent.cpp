#include "DDGIComponent.h"

#include "Log/Log.h"
#include "U_DDGI.sh"

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

CD_FORCEINLINE size_t GetTextureRaoDataSize(cd::Vec3f probeCount, size_t gridSize, size_t pixelSize)
{
    cd::Vec2f textureSize = cd::Vec2f(probeCount.y() * probeCount.z(), probeCount.x()) * gridSize;
    return static_cast<size_t>(textureSize.x() * textureSize.y()) * pixelSize / 8;
}

}

namespace engine
{

void DDGIComponent::ResetTextureRawData(cd::Vec3f probeCount)
{
    m_classificationRawData.clear();
    m_distanceRawData.clear();
    m_irradianceRawData.clear();
    m_relocationRawData.clear();

    // R32_FLOAT
    m_classificationRawData.resize(GetTextureRaoDataSize(probeCount, CLASSIFICATICON_GRID_SIZE, 32), 0);
    // R32G32_FLOAT
    m_distanceRawData.resize(GetTextureRaoDataSize(probeCount, DISTANCE_GRID_SIZE, 64), 0);
    // R16G16B16A16_FLOAT
    m_irradianceRawData.resize(GetTextureRaoDataSize(probeCount, IRRADIANCE_GRID_SIZE, 64), 0);
    // R16G16B16A16_FLOAT
    m_relocationRawData.resize(GetTextureRaoDataSize(probeCount, RELOCATION_GRID_SIZE, 64), 0);
}

void DDGIComponent::SetClassificationRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_classificationRawData);
}

void DDGIComponent::SetClassificationRawData(const std::shared_ptr<std::vector<uint8_t>>& classification)
{
    m_classificationRawData = cd::MoveTemp(*classification);
}

void DDGIComponent::SetDistanceRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_distanceRawData);
}

void DDGIComponent::SetDistanceRawData(const std::shared_ptr<std::vector<uint8_t>>& distance)
{
    m_distanceRawData = cd::MoveTemp(*distance);
}

void DDGIComponent::SetIrradianceRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_irradianceRawData);
}

void DDGIComponent::SetIrradianceRawData(const std::shared_ptr<std::vector<uint8_t>>& irrdiance)
{
    m_irradianceRawData = cd::MoveTemp(*irrdiance);
}

void DDGIComponent::SetRelocationRawData(const std::string& path)
{
    std::string absolutePath = GetBinaryFileRealPath(path);
    ReadTextureBinaryFile(absolutePath, m_relocationRawData);
}

void DDGIComponent::SetRelocationRawData(const std::shared_ptr<std::vector<uint8_t>>& relocation)
{
    m_relocationRawData = cd::MoveTemp(*relocation);
}

}
