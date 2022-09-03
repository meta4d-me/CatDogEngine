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

	RenderDataContext &&BgfxConsumer::GetRenderDataContext() {
		return std::move(m_renderDataContext);
	}

	void BgfxConsumer::Execute(const SceneDatabase *pSceneDatabase) {
		printf("DumpSceneDatabase:\n");
		printf("SceneName : %s\n", pSceneDatabase->GetName().c_str());
		printf("MeshCount : %u\n", pSceneDatabase->GetMeshCount());
		printf("MaterialCount : %u\n", pSceneDatabase->GetMaterialCount());

		const std::vector<cdtools::Mesh>     &meshes    = pSceneDatabase->GetMeshes();
		const std::vector<cdtools::Material> &materials = pSceneDatabase->GetMaterials();

		std::set<uint32_t> usedMaterialIDs;
		std::set<uint32_t> totalMaterialIDs;
		for (const cdtools::Material &material : materials) {
			totalMaterialIDs.insert(material.GetID().Data());
		}

		for (const cdtools::Mesh &mesh : meshes) {
			printf("\n\tMeshName : %s\n", mesh.GetName().c_str());
			printf("\t\tVertexCount : %u\n", mesh.GetVertexCount());
			printf("\t\tPolygonCount : %u\n", mesh.GetPolygonCount());
			MeshRenderData meshData;
			meshData.vertices.reserve(mesh.GetVertexCount());
			meshData.indices.reserve(mesh.GetPolygonCount() * 3);

			// 1. vertices
			for (uint32_t i = 0; i < mesh.GetVertexCount(); ++i) {
				PNTBUV vertexData;
				vertexData.x = mesh.GetVertexPosition(i).x();
				vertexData.y = mesh.GetVertexPosition(i).y();
				vertexData.z = mesh.GetVertexPosition(i).z();
				vertexData.normal = encodeNormalRgba8(
					mesh.GetVertexNormal(i)[0],
					mesh.GetVertexNormal(i)[1],
					mesh.GetVertexNormal(i)[2]
				);
				vertexData.tangent = encodeNormalRgba8(
					mesh.GetVertexTangent(i)[0],
					mesh.GetVertexTangent(i)[1],
					mesh.GetVertexTangent(i)[2]
				);
				vertexData.bitangent = encodeNormalRgba8(
					mesh.GetVertexBiTangent(i)[0],
					mesh.GetVertexBiTangent(i)[1],
					mesh.GetVertexBiTangent(i)[2]
				);
				vertexData.u = mesh.GetVertexUV(0)[i].x();
				vertexData.v = mesh.GetVertexUV(0)[i].y();

				meshData.vertices.emplace_back(std::move(vertexData));
			}

			// 2. indices
			for (uint32_t i = 0; i < mesh.GetPolygonCount(); ++i) {
				// Note that bgfx uses uint16_t as index buffer's value type,
				// so we need to cast uint32_t to smaller uint16_t here.
				uint16_t v0 = static_cast<uint16_t>(mesh.GetPolygon(i).v0.Data());
				uint16_t v1 = static_cast<uint16_t>(mesh.GetPolygon(i).v1.Data());
				uint16_t v2 = static_cast<uint16_t>(mesh.GetPolygon(i).v2.Data());

				meshData.indices.push_back(v0);
				meshData.indices.push_back(v1);
				meshData.indices.push_back(v2);
			}

			m_renderDataContext.meshRenderDataArray.emplace_back(std::move(meshData));

			// 3. materials
			uint32_t materialID = mesh.GetMaterialID().Data();
			const cdtools::Material &material = materials[materialID];
			printf("\t\tUsedMaterialName : %s\n", material.GetName().c_str());
			usedMaterialIDs.insert(materialID);

			static const std::vector<MaterialTextureType> materialsWillUse = {
				MaterialTextureType::BaseColor,
				MaterialTextureType::Normal,
				MaterialTextureType::Unknown,
			};

			MaterialRenderData materialData;
			for (const auto &type : materialsWillUse) {
				const auto textureID = material.GetTextureID(type);
				if (textureID.has_value()) {
					const std::string &texturePath = pSceneDatabase->GetTexture(textureID->Data()).GetPath();
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
	}
}
