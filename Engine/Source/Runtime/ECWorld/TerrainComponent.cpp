#include "TerrainComponent.h"

namespace cd
{

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

static std::optional<std::vector<std::byte>> GenerateElevationMap(uint16_t terrainWidth, uint16_t terrainDepth, float roughness, float minHeight, float maxHeight)
{
    assert(roughness > 0.0f);

    uint16_t RectSize = CalcNextPowerOfTwo(std::max(terrainWidth - 1, terrainDepth -1));
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

namespace engine
{

void TerrainComponent::InitElevationRawData()
{
    std::optional<std::vector<std::byte>> optMap = cd::GenerateElevationMap(m_width, m_depth, m_roughness, m_minHeight, m_maxHeight);//std::vector<std::byte>129U
    assert(optMap.has_value());
    SetElevationRawData(optMap.value());
}

void TerrainComponent::SetElevationRawDataAt(uint16_t x, uint16_t z, float data) {
	float temp = 30.0f;
	std::byte* bytePtr = reinterpret_cast<std::byte*>(&data);
	for (uint16_t i = 0U; i < sizeof(float); ++i)
	{
		m_elevationRawData[(z * m_width + x) * sizeof(float) + i] = bytePtr[i];
	}
}

float TerrainComponent::GetElevationRawDataAt(uint16_t x, uint16_t z)
{
	float data;
	memcpy(&data, &m_elevationRawData[(z * m_width + x) * sizeof(float)], sizeof(data));
	return data;
}

void TerrainComponent::SmoothElevationRawDataAround(uint16_t x, uint16_t z, int16_t brushSize, float power)
{
	float sum = 0;
	uint32_t count = 0;
    int16_t area_z, area_x, brush_x, brush_z;

	for (area_z = -brushSize; area_z < brushSize; ++area_z)
	{
		for (area_x = -brushSize; area_x < brushSize; ++area_x)
		{
			brush_x = x + area_x;
			if (brush_x < 0 || static_cast<uint16_t>(brush_x) >= m_width)
			{
				continue;
			}
			brush_z = z + area_z;
			if (brush_z < 0 || static_cast<uint16_t>(brush_z) >= m_depth)
			{
				continue;
			}
			sum += GetElevationRawDataAt(brush_x, brush_z);
			count++;
		}
	}

	float average = sum / count;
	for (area_z = -brushSize; area_z < brushSize; ++area_z)
	{
		for (area_x = -brushSize; area_x < brushSize; ++area_x)
		{
			brush_x = x + area_x;
			if (brush_x < 0 || static_cast<uint16_t>(brush_x) >= m_width)
			{
				continue;
			}
			brush_z = z + area_z;
			if (brush_z < 0 || static_cast<uint16_t>(brush_z) >= m_depth)
			{
				continue;
			}
			/*float a2 = (float)(area_x * area_x);
			float b2 = (float)(area_z * area_z);
			float brushAttn = (1.0f - bx::sqrt(a2 + b2) / brushSize) * (1 - power);*/
			float data = bx::lerp(GetElevationRawDataAt(brush_x, brush_z), average, 0.03f);//power + brushAttn);
			SetElevationRawDataAt(brush_x, brush_z, data);// );
		}
	}
}



}