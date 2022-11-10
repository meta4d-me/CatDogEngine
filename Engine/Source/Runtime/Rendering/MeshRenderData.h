#pragma once

#include "Scene/SceneDatabase.h"

#include <cassert>
#include <string>
#include <vector>

struct VertextData
{
	float m_position_x, m_position_y, m_position_z;
	float m_normal_x, m_normal_y, m_normal_z;
	float m_tangent_x, m_tangent_y, m_tangent_z;
	//float m_bitangent_x, m_bitangent_y, m_bitangent_z;
	float m_u, m_v;
};

struct PositionBarycentricCoordinatesVertext
{
	float m_position_x, m_position_y, m_position_z;
	// If we calculate face normal in fs, this vertext normal attribute is unnecessary.
	float m_normal_x, m_normal_y, m_normal_z;
	float m_barycentricCoordinates_x, m_barycentricCoordinates_y, m_barycentricCoordinates_z;
};
using EditorVertextData = PositionBarycentricCoordinatesVertext;

struct MeshRenderData
{
	constexpr uint32_t GetVerticesBufferLength() const
	{
		return static_cast<uint32_t>(vertices.size() * sizeof(decltype(vertices[0])));
	}

	constexpr uint32_t GetIndicesBufferLength() const
	{
		return static_cast<uint32_t>(indices.size() * sizeof(decltype(indices[0])));
	}

	std::vector<VertextData> vertices;
	std::vector<uint32_t> indices;
};

class MaterialRenderData
{
public:
	MaterialRenderData() = default;
	MaterialRenderData(const MaterialRenderData&) = default;
	MaterialRenderData& operator=(const MaterialRenderData&) = default;
	MaterialRenderData(MaterialRenderData&&) = default;
	MaterialRenderData& operator=(MaterialRenderData&&) = default;
	~MaterialRenderData() = default;

	const std::optional<std::string>& GetTextureName(const cdtools::MaterialTextureType type) const
	{
		switch (type)
		{
		case cdtools::MaterialTextureType::BaseColor:
			return m_baseColorName;
		case cdtools::MaterialTextureType::Normal:
			return m_normalName;
		case cdtools::MaterialTextureType::Roughness:
			return m_ORMName;
		default:
			assert("Invalid texture type!");
			return m_baseColorName;
		}
	}

	void SetTextureName(const cdtools::MaterialTextureType type, std::optional<std::string> name)
	{
		switch (type)
		{
		case cdtools::MaterialTextureType::BaseColor:
			m_baseColorName = std::move(name); break;
		case cdtools::MaterialTextureType::Normal:
			m_normalName = std::move(name); break;
		case cdtools::MaterialTextureType::Roughness:
			if(!m_ORMName.has_value())
			{
				m_ORMName = std::move(name);
			}
			break;
		default:
			assert("Set invalid texture type!\n"); break;
		}
	}

private:
	std::optional<std::string> m_baseColorName;
	std::optional<std::string> m_normalName;
	std::optional<std::string> m_ORMName;
};

struct RenderDataContext
{
	std::vector<MeshRenderData> meshRenderDataArray;
	std::vector<MaterialRenderData> materialRenderDataArray;

	cdtools::AABB sceneAABB;
};