#include "BgfxConsumer.h"

namespace
{

inline uint32_t toUnorm(float _value, float _scale)
{
	return uint32_t(std::round(std::clamp(_value, 0.0f, 1.0f) * _scale));
}

// RGBA8
inline void packRgba8(void* _dst, const float* _src)
{
	uint8_t* dst = (uint8_t*)_dst;
	dst[0] = uint8_t(toUnorm(_src[0], 255.0f));
	dst[1] = uint8_t(toUnorm(_src[1], 255.0f));
	dst[2] = uint8_t(toUnorm(_src[2], 255.0f));
	dst[3] = uint8_t(toUnorm(_src[3], 255.0f));
}

uint32_t encodeNormalRgba8(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f)
{
	const float src[] =
	{
		_x * 0.5f + 0.5f,
		_y * 0.5f + 0.5f,
		_z * 0.5f + 0.5f,
		_w * 0.5f + 0.5f,
	};
	uint32_t dst;
	packRgba8(&dst, src);
	return dst;
}

}

namespace cdtools
{
BgfxConsumer::BgfxConsumer(std::string filePath) :
	m_filePath(std::move(filePath)) {}

RenderDataContext&& BgfxConsumer::GetRenderDataContext() {
	return std::move(m_renderDataContext);
}

void BgfxConsumer::Execute(const SceneDatabase* pSceneDatabase) {
	printf("DumpSceneDatabase:\n");
	printf("SceneName : %s\n", pSceneDatabase->GetName().c_str());

	const cdtools::AABB& sceneAABB = pSceneDatabase->GetAABB();
	printf("AABB min : (%f, %f, %f)\n", sceneAABB.Min().x(), sceneAABB.Min().y(), sceneAABB.Min().z());
	printf("AABB max : (%f, %f, %f)\n", sceneAABB.Max().x(), sceneAABB.Max().y(), sceneAABB.Max().z());

	printf("MeshCount : %u\n", pSceneDatabase->GetMeshCount());
	printf("MaterialCount : %u\n", pSceneDatabase->GetMaterialCount());

	const std::vector<cdtools::Mesh>& meshes = pSceneDatabase->GetMeshes();
	const std::vector<cdtools::Material>& materials = pSceneDatabase->GetMaterials();

	std::set<uint32_t> usedMaterialIDs;
	std::set<uint32_t> totalMaterialIDs;
	for (const cdtools::Material& material : materials) {
		totalMaterialIDs.insert(material.GetID().Data());
	}

	assert(!meshes.empty() && "Meshes can not be empty.");
	//AABB sceneAabb;

	for (const cdtools::Mesh& mesh : meshes) {
		printf("\n\tMeshName : %s\n", mesh.GetName().c_str());
		printf("\t\tVertexCount : %u\n", mesh.GetVertexCount());
		printf("\t\tPolygonCount : %u\n", mesh.GetPolygonCount());

		// AABB
		//sceneAabb.Add(mesh.GetAABB());

		MeshRenderData meshData;
		meshData.vertices.reserve(mesh.GetVertexCount());
		meshData.indices.reserve(mesh.GetPolygonCount() * 3);

		// 1. vertices
		for (uint32_t i = 0; i < mesh.GetVertexCount(); ++i) {
			VertextData vertextData;

			std::memcpy(&vertextData.m_position_x, &mesh.GetVertexPosition(i), 3 * sizeof(float));
			std::memcpy(&vertextData.m_normal_x, &mesh.GetVertexNormal(i), 3 * sizeof(float));
			std::memcpy(&vertextData.m_tangent_x, &mesh.GetVertexTangent(i), 3 * sizeof(float));
			std::memcpy(&vertextData.m_bitangent_x, &mesh.GetVertexBiTangent(i), 3 * sizeof(float));
			std::memcpy(&vertextData.m_u, &mesh.GetVertexUV(0)[i].x(), 2 * sizeof(float));
			//std::memcpy(&vertextData.m_bc_x, &mesh.GetVertexBC(i), 3 * sizeof(float));

			meshData.vertices.emplace_back(std::move(vertextData));
		}

		// 2. indices
		for (uint32_t i = 0; i < mesh.GetPolygonCount(); ++i) {
			uint32_t v0 = mesh.GetPolygon(i)[0].Data();
			uint32_t v1 = mesh.GetPolygon(i)[1].Data();
			uint32_t v2 = mesh.GetPolygon(i)[2].Data();

			meshData.indices.push_back(v0);
			meshData.indices.push_back(v1);
			meshData.indices.push_back(v2);
		}

		m_renderDataContext.meshRenderDataArray.emplace_back(std::move(meshData));

		// 3. materials
		uint32_t materialID = mesh.GetMaterialID().Data();
		const cdtools::Material& material = materials[materialID];
		printf("\t\tUsedMaterialName : %s\n", material.GetName().c_str());
		usedMaterialIDs.insert(materialID);

		static const std::vector<MaterialTextureType> materialsWillUse = {
			MaterialTextureType::BaseColor,
			MaterialTextureType::Normal,
			MaterialTextureType::Unknown
		};

		MaterialRenderData materialData;
		for (const auto& type : materialsWillUse) {
			const auto textureID = material.GetTextureID(type);
			if (textureID.has_value()) {
				const std::string& texturePath = pSceneDatabase->GetTexture(textureID->Data()).GetPath();
				std::string textureName = texturePath.substr(texturePath.rfind('/') + 1, texturePath.rfind('.') - texturePath.rfind('/') - 1);
				printf("\t\t\ttextureName : %s\n", textureName.c_str());
				materialData.SetTextureName(type, std::move(textureName));
			}
			else {
				printf("\t\t\ttextureName : UnknownMaterial\n");
				materialData.SetTextureName(type, std::nullopt);
			}
		}
		m_renderDataContext.materialRenderDataArray.emplace_back(std::move(materialData));
	}

	//m_renderDataContext.sceneAabb = sceneAabb;
}

}