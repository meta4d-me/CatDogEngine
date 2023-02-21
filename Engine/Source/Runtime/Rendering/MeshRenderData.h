#pragma once

#include "Scene/SceneDatabase.h"

#include <bgfx/bgfx.h>
#include <cassert>
#include <string>
#include <vector>

struct PositionBarycentricCoordinatesVertext
{
	float m_position_x, m_position_y, m_position_z;
	// If we calculate face normal in fs, this vertices normal attribute is unnecessary.
	float m_normal_x, m_normal_y, m_normal_z;
	float m_barycentricCoordinates_x, m_barycentricCoordinates_y, m_barycentricCoordinates_z;
};
using EditorVertextData = PositionBarycentricCoordinatesVertext;

// TODO make this a sub-class of a more generic render data
class MeshRenderData
{
public:
	MeshRenderData() = default;
	MeshRenderData(const MeshRenderData& rhs) = delete;				// Prevent expensive copy
	MeshRenderData& operator=(const MeshRenderData& rhs) = delete;	// Prevent expensive copy
	~MeshRenderData() = default;

	MeshRenderData(MeshRenderData&& rhs) noexcept
		: m_rawVertices(std::move(rhs.m_rawVertices))
		, m_indices(std::move(rhs.m_indices))
	{
		m_vertexLayout.m_hash = rhs.m_vertexLayout.m_hash;
		m_vertexLayout.m_stride = rhs.m_vertexLayout.m_stride;

		size_t arrayLength = *(&rhs.m_vertexLayout.m_offset + 1) - rhs.m_vertexLayout.m_offset;
		std::memcpy(m_vertexLayout.m_offset, rhs.m_vertexLayout.m_offset, arrayLength * sizeof(uint16_t));

		arrayLength = *(&rhs.m_vertexLayout.m_attributes + 1) - rhs.m_vertexLayout.m_attributes;
		std::memcpy(m_vertexLayout.m_attributes, rhs.m_vertexLayout.m_attributes, arrayLength * sizeof(uint16_t));

		rhs.m_vertexLayout.m_stride = 0;
	}

	MeshRenderData& operator=(MeshRenderData&& rhs) noexcept
	{
		m_rawVertices = std::move(rhs.m_rawVertices);
		m_indices = std::move(rhs.m_indices);

		m_vertexLayout.m_hash = rhs.m_vertexLayout.m_hash;
		m_vertexLayout.m_stride = rhs.m_vertexLayout.m_stride;

		size_t arrayLength = *(&rhs.m_vertexLayout.m_offset + 1) - rhs.m_vertexLayout.m_offset;
		std::memcpy(m_vertexLayout.m_offset, rhs.m_vertexLayout.m_offset, arrayLength * sizeof(uint16_t));

		arrayLength = *(&rhs.m_vertexLayout.m_attributes + 1) - rhs.m_vertexLayout.m_attributes;
		std::memcpy(m_vertexLayout.m_attributes, rhs.m_vertexLayout.m_attributes, arrayLength * sizeof(uint16_t));

		rhs.m_vertexLayout.m_stride = 0;
	}

	uint32_t GetVerticesBufferLength() const
	{
		return static_cast<uint32_t>(m_rawVertices.size() * sizeof(std::byte));
	}

	uint32_t GetIndicesBufferLength() const
	{
		return static_cast<uint32_t>(m_indices.size() * sizeof(uint32_t));
	}

	bgfx::VertexLayout& GetVertexLayout() 
	{
		return m_vertexLayout;
	}

	const bgfx::VertexLayout& GetVertexLayout() const
	{
		return m_vertexLayout;
	}

	std::vector<std::byte>& GetRawVertices()
	{
		return m_rawVertices;
	}

	const std::vector<std::byte>& GetRawVertices() const
	{
		return m_rawVertices;
	}

	std::vector<uint32_t>& GetIndices()
	{
		return m_indices;
	}

	const std::vector<uint32_t>& GetIndices() const
	{
		return m_indices;
	}

private:
	bgfx::VertexLayout m_vertexLayout;
	std::vector<std::byte> m_rawVertices;	// array of structure based on vertexLayout
	std::vector<uint32_t> m_indices;

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

	const std::optional<std::string>& GetTextureName(const cd::MaterialTextureType type) const
	{
		switch (type)
		{
		case cd::MaterialTextureType::BaseColor:
			return m_baseColorName;
		case cd::MaterialTextureType::Normal:
			return m_normalName;
		case cd::MaterialTextureType::Metallic:
			return m_metalnessName;
		case cd::MaterialTextureType::Roughness:
			return m_ORMName;
		case cd::MaterialTextureType::Emissive:
			return m_emissiveName;
		case cd::MaterialTextureType::Occlusion:
			return m_aoName;
		default:
			assert("Invalid texture type!");
			return m_baseColorName;
		}
	}

	void SetTextureName(const cd::MaterialTextureType type, std::optional<std::string> name)
	{
		switch (type)
		{
		case cd::MaterialTextureType::BaseColor:
			m_baseColorName = std::move(name); 
			break;
		case cd::MaterialTextureType::Normal:
			m_normalName = std::move(name); 
			break;
		case cd::MaterialTextureType::Metallic:
			m_metalnessName = std::move(name);
			break;
		case cd::MaterialTextureType::Roughness:
			m_ORMName = std::move(name);
			break;
		case cd::MaterialTextureType::Emissive:
			m_emissiveName = std::move(name);
			break;
		case cd::MaterialTextureType::Occlusion:
			m_aoName = std::move(name);
			break;
		default:
			assert("Set invalid texture type!\n"); 
			break;
		}
	}

private:
	std::optional<std::string> m_baseColorName;
	std::optional<std::string> m_normalName;
	std::optional<std::string> m_metalnessName;
	std::optional<std::string> m_ORMName;
	std::optional<std::string> m_emissiveName;
	std::optional<std::string> m_aoName;
};

struct RenderDataContext
{
	std::vector<MeshRenderData> meshRenderDataArray;
	std::vector<MaterialRenderData> materialRenderDataArray;

	cd::AABB sceneAABB;
};