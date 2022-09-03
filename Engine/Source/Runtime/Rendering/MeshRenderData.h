#pragma once

#include "Scene/Material.h"

#include <assert.h>
#include <string>
#include <vector>

struct PositionNormalTangentBitangentTexCoord0Vertext
{
	float x;
	float y;
	float z;
	uint32_t normal;
	uint32_t tangent;
	uint32_t bitangent;
	float u;
	float v;
};
using PNTBUV = PositionNormalTangentBitangentTexCoord0Vertext;

struct MeshRenderData
{
	std::vector<PNTBUV> vertices;
	std::vector<uint16_t> indices;
};

class MaterialRenderData
{
public:
	MaterialRenderData() = default;
	MaterialRenderData(const MaterialRenderData &) = default;
	MaterialRenderData &operator=(const MaterialRenderData &) = default;
	MaterialRenderData(MaterialRenderData &&) = default;
	MaterialRenderData &operator=(MaterialRenderData &&) = default;

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

	const std::optional<std::string> &GetTextureName(const cdtools::MaterialTextureType type) const {
		switch (type)
		{
			case cdtools::MaterialTextureType::BaseColor:
				return m_baseColorName; break;
			case cdtools::MaterialTextureType::Normal:
				return m_normalName; break;
			case cdtools::MaterialTextureType::Unknown:
				return m_ORMName; break;
			default:
				return std::nullopt; break;
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
				assert(false && "Set invalid texture type!\n"); break;
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
};
