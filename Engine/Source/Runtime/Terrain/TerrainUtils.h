#include "Scene/VertexFormat.h"
#include "Scene/Mesh.h"

#include <cassert>
#include <optional>

namespace engine
{

std::optional<cd::Mesh> GenerateTerrainMesh(uint16_t width, uint16_t depth, const cd::VertexFormat& vertexFormat);
std::optional<std::vector<std::byte>> GenerateElevationMap(uint16_t terrainWidth, uint16_t terrainDepth, float roughness, float minHeight, float maxHeight);

}