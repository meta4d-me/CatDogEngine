#include "TerrainComponent.h"

namespace engine
{

void TerrainComponent::InitElevationRawData()
{
    std::optional<std::vector<std::byte>> optMap = GenerateElevationMap(m_texWidth, m_texDepth, m_roughness, m_minHeight, m_maxHeight);//std::vector<std::byte>129U
    assert(optMap.has_value());
    SetElevationRawData(optMap.value());
}

void TerrainComponent::SetElevationRawDataAt(uint16_t x, uint16_t z, float data) {
	float temp = 30.0f;
	std::byte* bytePtr = reinterpret_cast<std::byte*>(&data);
	for (uint16_t i = 0U; i < sizeof(float); ++i)
	{
		m_elevationRawData[(z * m_texWidth + x) * sizeof(float) + i] = bytePtr[i];
	}
}

float TerrainComponent::GetElevationRawDataAt(uint16_t x, uint16_t z)
{
	float data;
	memcpy(&data, &m_elevationRawData[(z * m_texWidth + x) * sizeof(float)], sizeof(data));
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
			if (brush_x < 0 || static_cast<uint16_t>(brush_x) >= m_texWidth)
			{
				continue;
			}
			brush_z = z + area_z;
			if (brush_z < 0 || static_cast<uint16_t>(brush_z) >= m_texDepth)
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
			if (brush_x < 0 || static_cast<uint16_t>(brush_x) >= m_texWidth)
			{
				continue;
			}
			brush_z = z + area_z;
			if (brush_z < 0 || static_cast<uint16_t>(brush_z) >= m_texDepth)
			{
				continue;
			}
			/*float a2 = (float)(area_x * area_x);
			float b2 = (float)(area_z * area_z);
			float brushAttn = (1.0f - bx::sqrt(a2 + b2) / brushSize) * (1 - power);*/
			float data = bx::lerp(GetElevationRawDataAt(brush_x, brush_z), average, 0.03f);//power + brushAttn);
			SetElevationRawDataAt(brush_x, brush_z, data);
		}
	}
}

void TerrainComponent::ScreenSpaceSmooth(float screenSpaceX, float screenSpaceY, cd::Matrix4x4 invProjMtx, cd::Matrix4x4 invViewMtx, cd::Vec3f camPos)
{
    cd::Vec4f ray_clip;
    ray_clip[0] = screenSpaceX;
    ray_clip[1] = screenSpaceY;
    ray_clip[2] = -1.0f;
    ray_clip[3] = 1.0f;

    cd::Vec4f ray_eye = invProjMtx * ray_clip;
    ray_eye[0] /= ray_eye[3];
    ray_eye[1] /= ray_eye[3];
    ray_eye[2] /= ray_eye[3];
    ray_eye[3] = 0.0f;

    cd::Vec4f ray_world = invViewMtx * ray_eye;
    cd::Vec3f rayDir = ray_world.xyz().Normalize();

    for (int i = 0; i < 1000; ++i)
    {
        camPos = camPos + rayDir;
        uint32_t posX = static_cast<uint32_t>(camPos.x());
        uint32_t posZ = static_cast<uint32_t>(camPos.z());
        if (posX < 0U || posX >= 129U || posZ < 0U || posZ >= 129U)
        {
            continue;
        }

        float terrainY = GetElevationRawDataAt(posX, posZ);
        if (camPos.y() < (terrainY))
        {
            SmoothElevationRawDataAround(posX, posZ, 10, 0.5f);
            break;
        }
    }
}

}