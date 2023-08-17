#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/Entity.h"
#include "Math/Box.hpp"
#include "Scene/Mesh.h"

#include <cstdint>
#include <vector>
#include <optional>
#include <bgfx/bgfx.h>
#include <bx/math.h>


namespace cd
{
//GeoMipmaping Algrithom to Generate Elevation Map
static std::optional<std::vector<std::byte>> GenerateElevationMap(uint16_t terrainWidth, uint16_t terrainDepth, float roughness, float minHeight, float maxHeight);
}
namespace engine
{

class World;

class TerrainComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("TerrainComponent");
		return className;
	}

public:
	TerrainComponent() = default;
	TerrainComponent(const TerrainComponent&) = default;
	TerrainComponent& operator=(const TerrainComponent&) = default;
	TerrainComponent(TerrainComponent&&) = default;
	TerrainComponent& operator=(TerrainComponent&&) = default;
	~TerrainComponent() = default;

	void SetWidth(const uint16_t width) { m_width = width; }
	uint16_t GetWidth() const { return m_width; }
	void SetDepth(const uint16_t depth) { m_depth = depth;}
	uint16_t GetDepth() const { return m_depth; }
	
	void InitElevationRawData();
	void SetElevationRawData(std::vector<std::byte> data) { m_elevationRawData = data; }
	const std::byte* GetElevationRawData() const { return m_elevationRawData.data(); }
	uint32_t GetElevationRawDataSize() const {return static_cast<uint32_t>(m_elevationRawData.size()); }

	void SetElevationRawDataAt(uint16_t x, uint16_t z, float data);
	float GetElevationRawDataAt(uint16_t x, uint16_t z);

	void SmoothElevationRawDataAround(uint16_t x, uint16_t z, int16_t brushSize, float power);
	
	void ScreenSpaceSmooth(float screenSpaceX, float screenSpaceY, cd::Matrix4x4 invProjMtx, cd::Matrix4x4 invViewMtx, cd::Vec3f camPos);

private:
	//height map input
	uint16_t m_width = 129U;//uint32_t is too big for width
	uint16_t m_depth = 129U;//
	float m_roughness = 1.55f;
	float m_minHeight = 0.0f;
	float m_maxHeight = 30.0f;
	
	//for patch wise generating
	//uint32_t m_PatchSize;

	//height map output
	std::vector<std::byte> m_elevationRawData;
};

}
