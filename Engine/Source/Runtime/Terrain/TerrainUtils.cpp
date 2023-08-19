#include "TerrainUtils.h"

namespace engine
{

std::optional<cd::Mesh> GenerateTerrainMesh(uint16_t width, uint16_t depth, const cd::VertexFormat& vertexFormat) 
{
    assert(vertexFormat.Contains(cd::VertexAttributeType::Position));

    std::vector<cd::Point> positions;
    positions.reserve(width * depth);
    for (uint16_t z = 0U; z < depth; z++) {
        for (uint16_t x = 0U; x < width; x++) {
            positions.push_back(cd::Point(x, 0, z));
        }
    }

    std::vector<cd::Polygon> polygons;
    uint32_t NumQuads = (width - 1) * (depth - 1);
    polygons.reserve(NumQuads * 2);

    for (uint16_t z = 1U; z < depth - 1; z += 2) {
        for (uint16_t x = 1U; x < width - 1; x += 2) {
            uint32_t IndexCenter = z * width + x;

            uint32_t IndexTemp1 = (z - 1) * width + x - 1;
            uint32_t IndexTemp2 = z * width + x - 1;

            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2 += width;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2++;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2++;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2 -= width;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2 -= width;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2--;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});

            IndexTemp1 = IndexTemp2;
            IndexTemp2--;
            polygons.push_back(cd::Polygon{IndexCenter, IndexTemp1, IndexTemp2});
        }
    }

    cd::Mesh mesh(static_cast<uint32_t>(positions.size()), static_cast<uint32_t>(polygons.size()));

    for (uint32_t i = 0U; i < positions.size(); ++i)
    {
        mesh.SetVertexPosition(i, positions[i]);
    }

    for (uint32_t i = 0U; i < polygons.size(); ++i)
    {
        mesh.SetPolygon(i, polygons[i]);
    }

    cd::VertexFormat meshVertexFormat;
    meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Position, cd::GetAttributeValueType<cd::Point::ValueType>(), cd::Point::Size);

    if (vertexFormat.Contains(cd::VertexAttributeType::Normal))
    {
        mesh.ComputeVertexNormals();
        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Normal, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
    }

    if (vertexFormat.Contains(cd::VertexAttributeType::UV))
    {
        mesh.SetVertexUVSetCount(1);
        for (uint32_t vertexIndex = 0U; vertexIndex < mesh.GetVertexCount(); ++vertexIndex)
        {
            const auto& position = mesh.GetVertexPosition(vertexIndex);
            mesh.SetVertexUV(0U, vertexIndex, cd::UV(position.x() / 4, position.z() / 4));
        }

        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::UV, cd::GetAttributeValueType<cd::UV::ValueType>(), cd::UV::Size);
    }

    if (vertexFormat.Contains(cd::VertexAttributeType::Tangent) || vertexFormat.Contains(cd::VertexAttributeType::Bitangent))
    {
        mesh.ComputeVertexTangents();
        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Tangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Bitangent, cd::GetAttributeValueType<cd::Direction::ValueType>(), cd::Direction::Size);
    }

    // Use VertexColor0 to present braycentric coordinates.
    if (vertexFormat.Contains(cd::VertexAttributeType::Color))
    {
        mesh.SetVertexColorSetCount(1U);
        meshVertexFormat.AddAttributeLayout(cd::VertexAttributeType::Color, cd::GetAttributeValueType<cd::Vec4f::ValueType>(), cd::Vec4f::Size);
    }

    mesh.SetVertexFormat(MoveTemp(meshVertexFormat));
    mesh.SetAABB(cd::AABB(cd::Point(0, 0, 0), cd::Point(width, 0, depth)));

    return mesh;
}

uint16_t CalcNextPowerOfTwo(uint16_t x)
{
    uint16_t ret = 1;
    if (x == 1) { return 2U; }
    while (ret < x) { ret = ret * 2; }
    return ret;
}

float RandomFloatRange(float Start, float End)
{
    float Delta = End - Start;
    float RandomValue = rand() * Delta + Start;
    return RandomValue;
}

std::optional<std::vector<std::byte>> GenerateElevationMap(uint16_t terrainWidth, uint16_t terrainDepth, float roughness, float minHeight, float maxHeight)
{
    assert(roughness > 0.0f);

    uint16_t RectSize = CalcNextPowerOfTwo(std::max(terrainWidth - 1, terrainDepth - 1));
    float CurElevation = static_cast<float>(RectSize) / 2.0f;
    float ElevationReduce = pow(2.0f, -roughness);
    std::vector<float> ElevationMap;
    ElevationMap.resize(terrainWidth * terrainDepth, 0);//* sizeof(int32_t)

    while (RectSize > 0U)
    {
        //DiamondStep
        int HalfRectSize = RectSize / 2;
        for (uint16_t z = 0U; z < terrainDepth; z += RectSize)
        {
            for (uint16_t x = 0U; x < terrainWidth; x += RectSize)
            {
                uint16_t next_x = (x + RectSize) % terrainWidth;
                uint16_t next_z = (z + RectSize) % terrainDepth;
                if (next_x < x)
                {
                    next_x = terrainWidth - 1;
                }
                if (next_z < z)
                {
                    next_z = terrainDepth - 1;
                }
                float TopLeft = ElevationMap[z * terrainWidth + x];
                float TopRight = ElevationMap[z * terrainWidth + next_x];
                float BottomLeft = ElevationMap[next_z * terrainWidth + x];
                float BottomRight = ElevationMap[next_z * terrainWidth + next_x];

                uint16_t mid_x = (x + HalfRectSize) % terrainWidth;
                uint16_t mid_z = (z + HalfRectSize) % terrainDepth;

                float RandValue = RandomFloatRange(CurElevation, -CurElevation);
                float MidPoint = (TopLeft + TopRight + BottomLeft + BottomRight) / 4.0f;

                ElevationMap[mid_z * terrainWidth + mid_x] = MidPoint + RandValue;
            }
        }

        //SquareStep
        for (uint16_t z = 0U; z < terrainDepth; z += RectSize)
        {
            for (uint16_t x = 0U; x < terrainWidth; x += RectSize)
            {
                uint16_t next_x = (x + RectSize) % terrainWidth;
                uint16_t next_z = (z + RectSize) % terrainDepth;

                if (next_x < x)
                {
                    next_x = terrainWidth - 1;
                }
                if (next_z < z)
                {
                    next_z = terrainDepth - 1;
                }

                uint32_t mid_x = (x + HalfRectSize) % terrainWidth;
                uint32_t mid_z = (z + HalfRectSize) % terrainDepth;
                uint32_t prev_mid_x = (x - HalfRectSize - 1 + terrainWidth) % terrainWidth;
                uint32_t prev_mid_z = (z - HalfRectSize - 1 + terrainDepth) % terrainDepth;

                float CurTopLeft = ElevationMap[z * terrainWidth + x];
                float CurTopRight = ElevationMap[z * terrainWidth + next_x];
                float CurCenter = ElevationMap[mid_z * terrainWidth + mid_x];
                float PrevYCenter = ElevationMap[prev_mid_z * terrainWidth + mid_x];
                float CurBotLeft = ElevationMap[next_z * terrainWidth + x];
                float PrevXCenter = ElevationMap[mid_z * terrainWidth + prev_mid_x];

                float CurLeftMid = (CurTopLeft + CurCenter + CurBotLeft + PrevXCenter) / 4.0f + RandomFloatRange(-CurElevation, CurElevation);
                float CurTopMid = (CurTopLeft + CurCenter + CurTopRight + PrevYCenter) / 4.0f + RandomFloatRange(-CurElevation, CurElevation);

                ElevationMap[z * terrainWidth + mid_x] = CurTopMid;
                ElevationMap[mid_z * terrainWidth + x] = CurLeftMid;
            }
        }
        RectSize /= 2;
        CurElevation *= ElevationReduce;

    }

    //mapping to [minHeight, maxHeight]
    float min = *std::min_element(ElevationMap.begin(), ElevationMap.end());
    float max = *std::max_element(ElevationMap.begin(), ElevationMap.end());
    for (auto& Elevation : ElevationMap)
    {
        Elevation = (Elevation - min) / (max - min) * (maxHeight - minHeight) + minHeight;
    }

    //float to std::byte
    std::vector<std::byte> outElevationMap;
    outElevationMap.reserve(terrainWidth * terrainDepth * sizeof(float));//
    for (auto& Elevation : ElevationMap)
    {
        std::byte* bytePtr = reinterpret_cast<std::byte*>(&Elevation);
        for (std::size_t i = 0; i < sizeof(float); ++i)
        {
            outElevationMap.push_back(bytePtr[i]);
        }
    }

    return outElevationMap;
}

}