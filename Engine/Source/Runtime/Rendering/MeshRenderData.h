#pragma once

#include "Scene/SceneDatabase.h"

#include <cassert>
#include <string>
#include <vector>

struct PositionNormalTangentBitangentTexCoord0BarycentricCoordinatesVertext
{
	float m_position_x, m_position_y, m_position_z;
	float m_normal_x, m_normal_y, m_normal_z;
	float m_tangent_x, m_tangent_y, m_tangent_z;
	float m_bitangent_x, m_bitangent_y, m_bitangent_z;
	float m_u, m_v;
	float m_bc_x, m_bc_y, m_bc_z;
};
using VertextData = PositionNormalTangentBitangentTexCoord0BarycentricCoordinatesVertext;

struct PositionBarycentricCoordinatesVertext {
	float m_position_x, m_position_y, m_position_z;
	// If we calculate face normal in fs, this vertext normal attribute is unnecessary.
	float m_normal_x, m_normal_y, m_normal_z;
	float m_barycentricCoordinates_x, m_barycentricCoordinates_y, m_barycentricCoordinates_z;
};
using EditorVertextData = PositionBarycentricCoordinatesVertext;

struct MeshRenderData
{
	constexpr uint32_t GetVerticesBufferLength() const {
		return static_cast<uint32_t>(vertices.size() * sizeof(decltype(vertices[0])));
	}

	constexpr uint32_t GetIndicesBufferLength() const {
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

	enum class MaterialType {
		PBR,
		decal,
		transparent,

		invalid,
	};

	void SetupMaterialType() {
		if (m_baseColorName.has_value()) {
			if (m_normalName.has_value() && m_ORMName.has_value()) {
				m_materialType = MaterialType::PBR;
			}
			else {
				m_materialType = MaterialType::decal;
			}
		}
		else {
			m_materialType = MaterialType::transparent;
		}
	}

	const MaterialType GetMaterialType() {
		if (m_materialType == MaterialType::invalid) {
			SetupMaterialType();
		}
		return m_materialType;
	}

	const std::optional<std::string>& GetTextureName(const cdtools::MaterialTextureType type) const {
		switch (type)
		{
		case cdtools::MaterialTextureType::BaseColor:
			return m_baseColorName;
		case cdtools::MaterialTextureType::Normal:
			return m_normalName;
		case cdtools::MaterialTextureType::Unknown:
			return m_ORMName;
		//case cdtools::MaterialTextureType::Error:
		default:
			assert("Invalid texture type!");
			return m_baseColorName;
		}
	}
	void SetTextureName(const cdtools::MaterialTextureType type, std::optional<std::string> name) {
		switch (type)
		{
		case cdtools::MaterialTextureType::BaseColor:
			m_baseColorName = std::move(name); break;
		case cdtools::MaterialTextureType::Normal:
			m_normalName = std::move(name); break;
		case cdtools::MaterialTextureType::Unknown:
			m_ORMName = std::move(name); break;
		default:
			assert("Set invalid texture type!\n"); break;
		}
	}

private:
	MaterialType m_materialType = MaterialType::invalid;

	std::optional<std::string> m_baseColorName;
	std::optional<std::string> m_normalName;
	std::optional<std::string> m_ORMName;
};

struct RenderDataContext
{
	std::vector<MeshRenderData> meshRenderDataArray;
	std::vector<MaterialRenderData> materialRenderDataArray;

	//cdtools::AABB sceneAabb;
};