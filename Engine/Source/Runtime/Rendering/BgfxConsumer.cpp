#include "BgfxConsumer.h"

#include "Log/Log.h"
#include "Scene/SceneDatabase.h"
#include "Scene/VertexFormat.h"
#include "Rendering/Utility/VertexLayoutUtility.h"

#include <cassert>

namespace
{

constexpr bgfx::Attrib::Enum AllAttribColorTypes[] = {
	bgfx::Attrib::Enum::Color0,
	bgfx::Attrib::Enum::Color1,
	bgfx::Attrib::Enum::Color2,
	bgfx::Attrib::Enum::Color3
};
constexpr uint32_t MAX_COLOR_COUNT = 4;

constexpr bgfx::Attrib::Enum AllAttribUVTypes[] = {
	bgfx::Attrib::Enum::TexCoord0,
	bgfx::Attrib::Enum::TexCoord1,
	bgfx::Attrib::Enum::TexCoord2,
	bgfx::Attrib::Enum::TexCoord3,
	bgfx::Attrib::Enum::TexCoord4,
	bgfx::Attrib::Enum::TexCoord5,
	bgfx::Attrib::Enum::TexCoord6,
	bgfx::Attrib::Enum::TexCoord7
};
constexpr uint32_t MAX_UV_COUNT = 8;

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

size_t GetSizeFromAttribType(const bgfx::AttribType::Enum attribType)
{
	switch (attribType) {
	case bgfx::AttribType::Enum::Float:
		return sizeof(float);
	case bgfx::AttribType::Enum::Half:
		return sizeof(float) >> 1;	// half the size of float
	case bgfx::AttribType::Enum::Uint8:
		return sizeof(uint8_t);
	case bgfx::AttribType::Enum::Int16:
		return sizeof(int16_t);
	case bgfx::AttribType::Enum::Uint10:
		CD_ENGINE_ERROR("Uint10 types are not supported!");
		assert(false);	// not supported
		return 0;
	default:
		CD_ENGINE_ERROR("Unknown attribute type!");
		assert(false);	// Unknown type!
		return 0;
	}
}

std::string MaterialTextureTypeToString(const cd::MaterialTextureType& materialType)
{
	switch (materialType)
	{
	case cd::MaterialTextureType::BaseColor:
		return "BaseColor";
	case cd::MaterialTextureType::Normal:
		return "Normal";
	case cd::MaterialTextureType::Metalness:
		return "Metalness";
	case cd::MaterialTextureType::Roughness:
		return "Roughness";
	case cd::MaterialTextureType::Emissive:
		return "Emissive";
	case cd::MaterialTextureType::AO:
		return "AO";
	default:
		return "Invalid Material Type!";
	}
}

constexpr cd::MaterialTextureType PossibleTextureTypes[] = {
	cd::MaterialTextureType::BaseColor,
	cd::MaterialTextureType::Normal,
	cd::MaterialTextureType::Metalness,
	cd::MaterialTextureType::Roughness,
	cd::MaterialTextureType::Emissive,
	cd::MaterialTextureType::AO
};

float bytesToFloat(const uint8_t* bytes)
{
	static bool isBigEndian = std::endian::native == std::endian::big;
	float result;
	uint8_t* result_ptr = reinterpret_cast<uint8_t*>(&result);
	if (isBigEndian)
	{
		result_ptr[3] = bytes[0];
		result_ptr[2] = bytes[1];
		result_ptr[1] = bytes[2];
		result_ptr[0] = bytes[3];
	}
	else
	{
		result_ptr[3] = bytes[3];
		result_ptr[2] = bytes[2];
		result_ptr[1] = bytes[1];
		result_ptr[0] = bytes[0];
	}
	return result;
}

void sanityCheck(const std::byte* dataPtr, const uint16_t offset, const float* inX, const float* inY, const float* inZ, const float* inW)
{
	if (inX != nullptr)
	{
		const float x = bytesToFloat(reinterpret_cast<const uint8_t*>(&dataPtr[offset + 0]));
		assert(x == *inX);
	}
	if (inY != nullptr)
	{
		const float y = bytesToFloat(reinterpret_cast<const uint8_t*>(&dataPtr[offset + 4]));
		assert(y == *inY);
	}
	if (inZ != nullptr)
	{
		const float z = bytesToFloat(reinterpret_cast<const uint8_t*>(&dataPtr[offset + 8]));
		assert(z == *inZ);
	}
	if (inW != nullptr)
	{
		const float w = bytesToFloat(reinterpret_cast<const uint8_t*>(&dataPtr[offset + 12]));
		assert(w == *inW);
	}
}

}

namespace engine
{

void BgfxConsumer::Execute(const cd::SceneDatabase* pSceneDatabase)
{
	CD_ENGINE_INFO("Loading Scene: {0}", pSceneDatabase->GetName());
	CD_ENGINE_TRACE("MeshCount : {0}", pSceneDatabase->GetMeshCount());
	CD_ENGINE_TRACE("MaterialCount : {0}", pSceneDatabase->GetMaterialCount());

	const cd::AABB& sceneAABB = pSceneDatabase->GetAABB();
	CD_ENGINE_TRACE("Scene AABB min: {0}, max: {1}", sceneAABB.Min(), sceneAABB.Max());
	m_renderDataContext.sceneAABB = sceneAABB;

	ConvertMeshesFromScene(*pSceneDatabase, m_renderDataContext.meshRenderDataArray);
	GetMaterialsFromScene(*pSceneDatabase, m_renderDataContext.materialRenderDataArray);
}

void BgfxConsumer::ConvertMeshesFromScene(const cd::SceneDatabase& sceneDatabase, std::vector<MeshRenderData>& outLoadedMeshes) const
{
	const std::vector<cd::Mesh>& meshes = sceneDatabase.GetMeshes();
	if (meshes.empty())
	{
		CD_ENGINE_ERROR("No meshes found for scene: {0}", sceneDatabase.GetName());
		return;
	}
	CD_ENGINE_TRACE("Loading {0} meshes", meshes.size());
	for (const cd::Mesh& mesh : meshes)
	{
		CD_ENGINE_TRACE("\tMeshName : {}", mesh.GetName());
		CD_ENGINE_TRACE("\t\tVertexCount : {}", mesh.GetVertexCount());
		CD_ENGINE_TRACE("\t\tPolygonCount : {}", mesh.GetPolygonCount());

		outLoadedMeshes.emplace_back();
		MeshRenderData& meshData = outLoadedMeshes.back();

		// Convert vertex formats
 		bgfx::VertexLayout& vertexLayout = meshData.GetVertexLayout();
		VertexLayoutUtility::CreateVertexLayout(vertexLayout, mesh.GetVertexFormat().GetVertexLayout(), true);
		
		// Convert vertices
		std::vector<std::byte>& rawVertices = meshData.GetRawVertices(); 
		rawVertices.resize(mesh.GetVertexCount() * vertexLayout.getStride());
		std::byte* currentDataPtr = rawVertices.data();
		bgfx::AttribType::Enum attribType = bgfx::AttribType::Enum::Count;
		uint8_t attribNum = 0;
		bool normalized = false;
		bool asInt = false;
		uint16_t attribOffset = 0;
		size_t attribTypeSize = 0;
		for (uint32_t vertexIndex = 0; vertexIndex < mesh.GetVertexCount(); ++vertexIndex)
		{
			if (vertexLayout.has(bgfx::Attrib::Enum::Position))
			{
				attribOffset = vertexLayout.getOffset(bgfx::Attrib::Enum::Position);
				vertexLayout.decode(bgfx::Attrib::Enum::Position, attribNum, attribType, normalized, asInt);
				attribTypeSize = GetSizeFromAttribType(attribType);
				std::memcpy(&currentDataPtr[attribOffset], mesh.GetVertexPosition(vertexIndex).Begin(), attribNum * attribTypeSize);
				sanityCheck(currentDataPtr, attribOffset, mesh.GetVertexPosition(vertexIndex).Begin(), mesh.GetVertexPosition(vertexIndex).Begin() + 1, mesh.GetVertexPosition(vertexIndex).Begin()+2, nullptr);
			}

			if (vertexLayout.has(bgfx::Attrib::Enum::Normal))
			{
				attribOffset = vertexLayout.getOffset(bgfx::Attrib::Enum::Normal);
				vertexLayout.decode(bgfx::Attrib::Enum::Normal, attribNum, attribType, normalized, asInt);
				attribTypeSize = GetSizeFromAttribType(attribType);
				std::memcpy(&currentDataPtr[attribOffset], mesh.GetVertexNormal(vertexIndex).Begin(), attribNum * attribTypeSize);
				sanityCheck(currentDataPtr, attribOffset, mesh.GetVertexNormal(vertexIndex).Begin(), mesh.GetVertexNormal(vertexIndex).Begin() + 1, mesh.GetVertexNormal(vertexIndex).Begin() + 2, nullptr);
			}

			if (vertexLayout.has(bgfx::Attrib::Enum::Tangent))
			{
				attribOffset = vertexLayout.getOffset(bgfx::Attrib::Enum::Tangent);
				vertexLayout.decode(bgfx::Attrib::Enum::Tangent, attribNum, attribType, normalized, asInt);
				attribTypeSize = GetSizeFromAttribType(attribType);
				std::memcpy(&currentDataPtr[attribOffset], mesh.GetVertexTangent(vertexIndex).Begin(), attribNum * attribTypeSize);
				sanityCheck(currentDataPtr, attribOffset, mesh.GetVertexTangent(vertexIndex).Begin(), mesh.GetVertexTangent(vertexIndex).Begin() + 1, mesh.GetVertexTangent(vertexIndex).Begin() + 2, nullptr);
			}

			if (vertexLayout.has(bgfx::Attrib::Enum::Bitangent))
			{
				attribOffset = vertexLayout.getOffset(bgfx::Attrib::Enum::Bitangent);
				vertexLayout.decode(bgfx::Attrib::Enum::Bitangent, attribNum, attribType, normalized, asInt);
				attribTypeSize = GetSizeFromAttribType(attribType);
				std::memcpy(&currentDataPtr[attribOffset], mesh.GetVertexBiTangent(vertexIndex).Begin(), attribNum * attribTypeSize);
				sanityCheck(currentDataPtr, attribOffset, mesh.GetVertexBiTangent(vertexIndex).Begin(), mesh.GetVertexBiTangent(vertexIndex).Begin() + 1, mesh.GetVertexBiTangent(vertexIndex).Begin() + 2, nullptr);
			}

			assert(MAX_COLOR_COUNT >= cd::MaxColorSetNumber);
			for (uint32_t i = 0; i < cd::MaxColorSetNumber; ++i)
			{
				const bgfx::Attrib::Enum color = AllAttribColorTypes[i];
				if (!vertexLayout.has(color))
				{
					continue;
				}
				attribOffset = vertexLayout.getOffset(color);
				vertexLayout.decode(color, attribNum, attribType, normalized, asInt);
				attribTypeSize = GetSizeFromAttribType(attribType);
				std::memcpy(&currentDataPtr[attribOffset], mesh.GetVertexColor(i)[vertexIndex].Begin(), attribNum * attribTypeSize);
				sanityCheck(currentDataPtr, attribOffset, mesh.GetVertexColor(i)[vertexIndex].Begin(), mesh.GetVertexColor(i)[vertexIndex].Begin() + 1, mesh.GetVertexColor(i)[vertexIndex].Begin() + 2, mesh.GetVertexColor(i)[vertexIndex].Begin() + 3);
			}

			assert(MAX_UV_COUNT >= cd::MaxUVSetNumber);
			for (uint32_t i = 0; i < cd::MaxUVSetNumber; ++i)
			{
				const bgfx::Attrib::Enum uv = AllAttribUVTypes[i];
				if (!vertexLayout.has(uv))
				{
					continue;
				}
				attribOffset = vertexLayout.getOffset(uv);
				vertexLayout.decode(uv, attribNum, attribType, normalized, asInt);
				attribTypeSize = GetSizeFromAttribType(attribType);
				std::memcpy(&currentDataPtr[attribOffset], mesh.GetVertexUV(i)[vertexIndex].Begin(), attribNum * attribTypeSize);
				sanityCheck(currentDataPtr, attribOffset, mesh.GetVertexUV(i)[vertexIndex].Begin(), mesh.GetVertexUV(i)[vertexIndex].Begin() + 1, nullptr, nullptr);
			}

			// Advance currentDataPtr by stride
			currentDataPtr += vertexLayout.getStride();
		}
		assert(rawVertices.size() == mesh.GetVertexCount() * vertexLayout.getStride());

		meshData.GetIndices().reserve(mesh.GetPolygonCount() * 3);	// We store triangles for polygons
		for (uint32_t i = 0; i < mesh.GetPolygonCount(); ++i)
		{
			meshData.GetIndices().push_back(mesh.GetPolygon(i)[0].Data());
			meshData.GetIndices().push_back(mesh.GetPolygon(i)[1].Data());
			meshData.GetIndices().push_back(mesh.GetPolygon(i)[2].Data());
		}
	}
}

void BgfxConsumer::GetMaterialsFromScene(const cd::SceneDatabase& sceneDatabase, std::vector<MaterialRenderData>& outLoadedMaterials) const
{
	const std::vector<cd::Mesh>& meshes = sceneDatabase.GetMeshes();
	uint32_t index = 0;
	for (const cd::Material& material : sceneDatabase.GetMaterials())
	{
		// Materials
		CD_ENGINE_TRACE("\t\tMaterial Name: {0}", material.GetName());
		outLoadedMaterials.emplace_back();
		MaterialRenderData& materialData = outLoadedMaterials.back();
		for (const cd::MaterialTextureType& textureType : PossibleTextureTypes)
		{
			CD_ENGINE_TRACE("\t\t\tMaterial Type: {0}", MaterialTextureTypeToString(textureType).c_str());
			const std::optional<cd::TextureID>& textureID = material.GetTextureID(textureType);
			if (textureID.has_value())
			{
				const std::string& texturePath = sceneDatabase.GetTexture(textureID->Data()).GetPath();
				std::string textureName = texturePath.substr(texturePath.rfind('/') + 1, texturePath.rfind('.') - texturePath.rfind('/') - 1);
				CD_ENGINE_TRACE("\t\t\tTexture Name: {0}", textureName.c_str());
				materialData.SetTextureName(textureType, std::move(textureName));
			}
			else
			{
				CD_ENGINE_WARN("\t\t\tTexture Name: UnknownMaterial");
				materialData.SetTextureName(textureType, std::nullopt);
			}
		}
		assert(index < meshes.size());
		const cd::Mesh& mesh = meshes[index];
		// Material ID must match mesh's ID
		assert(mesh.GetMaterialID().Data() == index);
		++index;
	}
}

} // namespace cdtools