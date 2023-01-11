#include "ECWorldConsumer.h"

#include "ECWorld/ComponentsStorage.hpp"
#include "ECWorld/HierarchyComponent.h"
#include "ECWorld/MaterialComponent.h"
#include "ECWorld/NameComponent.h"
#include "ECWorld/World.h"
#include "ECWorld/StaticMeshComponent.h"
#include "ECWorld/TransformComponent.h"
#include "Material/MaterialType.h"
#include "Rendering/RenderContext.h"
#include "Scene/Material.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/SceneDatabase.h"
#include "Scene/VertexFormat.h"

#include <filesystem>
#include <format>
#include <map>

namespace editor
{

void ECWorldConsumer::SetSceneDatabaseIDs(uint32_t meshID)
{
	m_meshMinID = meshID;
}

void ECWorldConsumer::Execute(const cd::SceneDatabase* pSceneDatabase)
{
	assert(pSceneDatabase->GetMeshCount() > 0);

	std::map<cd::TransformID::ValueType, engine::Entity> mapTransformIDToEntities;

	for (const auto& transform : pSceneDatabase->GetTransforms())
	{
		for (cd::MeshID meshID : transform.GetMeshIDs())
		{
			const auto& mesh = pSceneDatabase->GetMesh(meshID.Data());
			if (m_meshMinID > mesh.GetID().Data())
			{
				// The SceneDatabase can be reused when we import assets multiple times.
				// So we need to filter meshes which was generated in the past.
				continue;
			}

			assert(mesh.GetVertexCount() > 0 && mesh.GetPolygonCount() > 0);

			engine::Entity meshEntity = m_pWorld->CreateEntity();

			engine::NameComponent& nameComponent = m_pWorld->CreateComponent<engine::NameComponent>(meshEntity);
			nameComponent.SetName(mesh.GetName());

			engine::HierarchyComponent& hierarchyComponent = m_pWorld->CreateComponent<engine::HierarchyComponent>(meshEntity);
			cd::TransformID parentTransformID = transform.GetParentID();
			bool hasParent = parentTransformID.Data() != cd::TransformID::InvalidID;
			if (hasParent)
			{
				// It requires that we parse Transform nodes from Top to Down as a Depth-First Search.
				assert(mapTransformIDToEntities.contains(parentTransformID.Data()));
				hierarchyComponent.SetParentEntity(mapTransformIDToEntities[parentTransformID.Data()]);
			}

			engine::TransformComponent& transformComponent = m_pWorld->CreateComponent<engine::TransformComponent>(meshEntity);
			transformComponent.SetTranslation(cd::Vec3f(0.0f, 0.0f, 0.0f));
			transformComponent.SetRotation(cd::Quaternion::FromAxisAngle(cd::Vec3f(0.0f, 0.0f, 1.0f), 0.0f));
			transformComponent.SetScale(cd::Vec3f(1.0f, 1.0f, 1.0f));

			engine::StaticMeshComponent& staticMeshComponent = m_pWorld->CreateComponent<engine::StaticMeshComponent>(meshEntity);
			staticMeshComponent.SetMeshData(&mesh);
			staticMeshComponent.SetRequiredVertexFormat(&m_pStandardMaterialType->GetRequiredVertexFormat());

			cd::MaterialID meshMaterialID = mesh.GetMaterialID();
			if (meshMaterialID.IsValid())
			{
				engine::MaterialComponent& materialComponent = m_pWorld->CreateComponent<engine::MaterialComponent>(meshEntity);

				const cd::Material& materialData = pSceneDatabase->GetMaterial(mesh.GetMaterialID().Data());
				materialComponent.SetMaterialData(&materialData);
				
				bool missRequiredTextures = false;
				for (cd::MaterialTextureType requiredTextureType : m_pStandardMaterialType->GetRequiredTextureTypes())
				{
					std::optional<cd::TextureID> optTexture = materialData.GetTextureID(requiredTextureType);
					if (!optTexture.has_value())
					{
						missRequiredTextures = true;
						break;
					}

					const cd::Texture& requiredTexture = pSceneDatabase->GetTexture(optTexture.value().Data());
					materialComponent.AddTextureData(&requiredTexture);
				}

				if (missRequiredTextures)
				{
					// Missed required textures are unexpected behavior which will have a notification in the apperance with a special color.
					bgfx::ProgramHandle shadingProgram = m_pRenderContext->CreateProgram("MissingTextures", m_pStandardMaterialType->GetVertexShaderName(), "fs_missing_textures.bin");
					materialComponent.SetShadingProgram(shadingProgram.idx);
				}
				else
				{
					// Expected textures are ready to build. Add more optional texture data.
					for (cd::MaterialTextureType optionalTextureType : m_pStandardMaterialType->GetOptionalTextureTypes())
					{
						std::optional<cd::TextureID> optTexture = materialData.GetTextureID(optionalTextureType);
						if (!optTexture.has_value())
						{
							continue;
						}

						const cd::Texture& optionalTexture = pSceneDatabase->GetTexture(optTexture.value().Data());
						materialComponent.AddTextureData(&optionalTexture);

						bgfx::ProgramHandle shadingProgram = m_pRenderContext->CreateProgram(m_pStandardMaterialType->GetMaterialName(), m_pStandardMaterialType->GetVertexShaderName(), m_pStandardMaterialType->GetFragmentShaderName());
						materialComponent.SetShadingProgram(shadingProgram.idx);
					}
				}
			}

			m_meshEntities.push_back(meshEntity);
		}
	}
}

}