#include "Inspector.h"

#include "Rendering/RenderContext.h"
#include "Rendering/ShaderCollections.h"
#include "Graphics/GraphicsBackend.h"
#include "ImGui/ImGuiUtils.hpp"
#include "Path/Path.h"

namespace details
{

template<typename Component>
void UpdateComponentWidget(engine::SceneWorld* pSceneWorld, engine::Entity entity) {}

template<>
void UpdateComponentWidget<engine::CollisionMeshComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pCollisionMesh = pSceneWorld->GetCollisionMeshComponent(entity);
	if (!pCollisionMesh)
	{
		return;
	}

	bool isHeaderOpen = ImGui::CollapsingHeader("Collision Mesh Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isHeaderOpen)
	{
		ImGuiUtils::ImGuiBoolProperty("Debug Draw", pCollisionMesh->IsDebugDrawEnable());
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::NameComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pNameComponent = pSceneWorld->GetNameComponent(entity);
	if (!pNameComponent)
	{
		return;
	}

	bool isHeaderOpen = ImGui::CollapsingHeader("Name Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isHeaderOpen)
	{
		ImGuiUtils::ImGuiStringProperty("Name", pNameComponent->GetNameForWrite());
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::TransformComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pTransformComponent = pSceneWorld->GetTransformComponent(entity);
	if (!pTransformComponent)
	{
		return;
	}

	bool isHeaderOpen = ImGui::CollapsingHeader("Transform Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isHeaderOpen)
	{
		if (ImGuiUtils::ImGuiTransformProperty("Transform", pTransformComponent->GetTransform()))
		{
			pTransformComponent->Dirty();
			pTransformComponent->Build();
		}
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::StaticMeshComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pStaticMeshComponent = pSceneWorld->GetStaticMeshComponent(entity);
	if (!pStaticMeshComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("StaticMesh Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		ImGuiUtils::ImGuiStringProperty("Vertex Count", std::to_string(pStaticMeshComponent->GetVertexCount()));
		ImGuiUtils::ImGuiStringProperty("Triangle Count", std::to_string(static_cast<uint32_t>(pStaticMeshComponent->GetPolygonCount())));

		if (!pStaticMeshComponent->IsProgressiveMeshValid())
		{
			if (ImGui::Button(reinterpret_cast<const char*>("Build ProgressiveMesh")))
			{
				pStaticMeshComponent->BuildProgressiveMeshData();

			}
		}
		else
		{
			ImGuiUtils::ImGuiFloatProperty("LOD Percent", pStaticMeshComponent->GetProgressiveMeshReductionPercent(),
				cd::Unit::None, 0.001f, 1.0f, false, 0.001f);
			ImGuiUtils::ImGuiIntProperty("Target VertexCount", reinterpret_cast<int&>(pStaticMeshComponent->GetProgressiveMeshTargetVertexCount()),
				cd::Unit::None, 0, static_cast<int>(pStaticMeshComponent->GetOriginVertexCount()), false, 1);
		}
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::BlendShapeComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pBlendShapeComponent = pSceneWorld->GetBlendShapeComponent(entity);
	if (!pBlendShapeComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("BlendShape Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		// Parameters
		uint32_t morphCount = pBlendShapeComponent->GetMorphCount();
		std::vector<float>& weights = pBlendShapeComponent->GetWeights();
		for (uint32_t morphIndex = 0; morphIndex < morphCount; ++morphIndex)
		{
			const auto* pMorph = pBlendShapeComponent->GetMorphData(morphIndex);
			float weightI = weights[morphIndex];
			if (ImGuiUtils::ImGuiFloatProperty(pMorph->GetName(), weights[morphIndex], cd::Unit::None, 0.0f, 1.0f))//, false, 0.1f
			{
				pBlendShapeComponent->AddNeedUpdate(morphIndex, weightI);
			}
		}
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::MaterialComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pMaterialComponent = pSceneWorld->GetMaterialComponent(entity);
	if (!pMaterialComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("Material Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		/*
		std::vector<std::string> fileNames;
		std::filesystem::path dirPath{ CDPROJECT_RESOURCES_ROOT_PATH };
		dirPath /= "test";
		std::filesystem::path frontPath{ "test" };
		engine::RenderContext* pRenderContext = engine::ImGuiBaseLayer::GetRenderContext();

		std::vector<std::string> texturePaths;
		for (const auto& it : std::filesystem::directory_iterator(dirPath))
		{
			std::string fileName = it.path().filename().string();
			std::string fullpath = (frontPath / fileName).string();
			pRenderContext->CreateTexture(fullpath.c_str());

			fileNames.emplace_back(cd::MoveTemp(fileName));
			texturePaths.emplace_back(cd::MoveTemp(fullpath));
		}


		static int currentItem = 0;
		if (ImGui::BeginCombo("Select Texture", fileNames[currentItem].c_str()))
		{
			for (int i = 0; i < fileNames.size(); ++i)
			{
				bool isSelected = (currentItem == i);
				if (ImGui::Selectable(fileNames[i].c_str(), isSelected))
				{
					currentItem = i;
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		bgfx::TextureHandle textureHandle = pRenderContext->GetTexture(engine::StringCrc(texturePaths[currentItem].c_str()));
		//sparater
		ImGui::Separator();
		ImGui::Image(ImTextureID(textureHandle.idx), ImVec2(64, 64));
		*/
		ImGui::Separator();
		ImGuiUtils::ImGuiStringProperty("Name", pMaterialComponent->GetName());

		// Parameters
		ImGui::Separator();
		
		{
			bool isOpen = ImGui::CollapsingHeader("Render States", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::Separator();

			if (isOpen)
			{
				// TODO : generic cull mode.
				ImGuiUtils::ImGuiBoolProperty("TwoSided", pMaterialComponent->GetTwoSided());
				ImGuiUtils::ImGuiEnumProperty("BlendMode", pMaterialComponent->GetBlendMode());
			}

			ImGui::Separator();
			ImGui::PopStyleVar();
		}

		if (cd::BlendMode::Mask == pMaterialComponent->GetBlendMode())
		{
			ImGuiUtils::ImGuiFloatProperty("AlphaCutOff", pMaterialComponent->GetAlphaCutOff(), cd::Unit::None, 0.0f, 1.0f);
		}

		// Textures
		for (int textureTypeValue = 0; textureTypeValue < nameof::enum_count<cd::MaterialTextureType>(); ++textureTypeValue)
		{
			auto textureType = static_cast<cd::MaterialTextureType>(textureTypeValue);
			auto pPropertyGroup = pMaterialComponent->GetPropertyGroup(textureType);
			if (!pPropertyGroup)
			{
				continue;
			}

			const char* pTextureType = nameof::nameof_enum(static_cast<cd::MaterialTextureType>(textureTypeValue)).data();
			bool isOpen = ImGui::CollapsingHeader(pTextureType, ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::Separator();

			if (isOpen)
			{
				ImGui::PushID(textureTypeValue);

				auto& textureInfo = pPropertyGroup->textureInfo;
				if (bgfx::kInvalidHandle != textureInfo.textureHandle)
				{
					ImGui::Image(reinterpret_cast<ImTextureID>(textureInfo.textureHandle), ImVec2(64, 64));
				}
				else
				{
					// Draw a black square with text here.
					ImVec2 currentPos = ImGui::GetCursorScreenPos();
					ImGui::GetWindowDrawList()->AddRectFilled(currentPos, ImVec2{ currentPos.x + 64, currentPos.y + 64 }, IM_COL32(0, 0, 0, 255));
					ImGui::SetCursorScreenPos(ImVec2{ currentPos.x, currentPos.y + 27});
					ImGui::SetWindowFontScale(0.55f);
					ImGui::Text("No Resources");
					ImGui::SetWindowFontScale(1.0f);
					ImGui::SetCursorScreenPos(ImVec2{ currentPos.x, currentPos.y + 66});
				}

				ImGuiUtils::ImGuiVectorProperty("UV Offset", textureInfo.GetUVOffset(), cd::Unit::None, cd::Vec2f::Zero(), cd::Vec2f::One(), false, 0.01f);
				ImGuiUtils::ImGuiVectorProperty("UV Scale", textureInfo.GetUVScale());
				ImGuiUtils::ImGuiBoolProperty("Use texture", pPropertyGroup->useTexture);

				if (pPropertyGroup->useTexture)
				{
					pMaterialComponent->ActivateShaderFeature(engine::MaterialTextureTypeToShaderFeature.at(textureType));
				}
				else
				{
					pMaterialComponent->DeactivateShaderFeature(engine::MaterialTextureTypeToShaderFeature.at(textureType));
				}

				if (cd::MaterialTextureType::BaseColor == textureType)
				{
					ImGuiUtils::ColorPickerProperty("Factor", *(pMaterialComponent->GetFactor<cd::Vec3f>(textureType)));
				}
				else if (cd::MaterialTextureType::Occlusion == textureType ||
					cd::MaterialTextureType::Metallic == textureType ||
					cd::MaterialTextureType::Roughness == textureType)
				{
					ImGuiUtils::ImGuiFloatProperty("Factor", *(pMaterialComponent->GetFactor<float>(textureType)), cd::Unit::None, 0.0f, 1.0f, false, 0.01f);
				}
				else if (cd::MaterialTextureType::Emissive == textureType)
				{
					cd::Vec4f& emissiveColorAndFactor = *(pMaterialComponent->GetFactor<cd::Vec4f>(textureType));
					cd::Vec3f emissiveColor{ emissiveColorAndFactor.x(), emissiveColorAndFactor.y(), emissiveColorAndFactor.z() };
					float emissiveFactor = emissiveColorAndFactor.w();
					ImGuiUtils::ImGuiVectorProperty("Color", emissiveColor, cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One(), false, 0.01f);
					ImGuiUtils::ImGuiFloatProperty("Factor", emissiveFactor, cd::Unit::None, 0.0f, 10000.0f, false, 0.1f);
					emissiveColorAndFactor = cd::Vec4f{ emissiveColor.x(), emissiveColor.y(), emissiveColor.z(), emissiveFactor };
				}

				ImGui::PopID();
			}

			ImGui::Separator();
			ImGui::PopStyleVar();
		}

		// Shaders
		{
			bool isOpen = ImGui::CollapsingHeader("Shader", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::Separator();

			if (isOpen)
			{
				const auto& shaderProgramName = pMaterialComponent->GetShaderProgramName();
				ImGuiUtils::ImGuiStringProperty("Shader Program", shaderProgramName);

				engine::RenderContext* pRenderContext = static_cast<engine::RenderContext*>(ImGui::GetIO().BackendRendererUserData);
				for (const auto& shaderFileName : pRenderContext->GetShaderCollections()->GetShaders(engine::StringCrc{ shaderProgramName }))
				{
					ImGuiUtils::ImGuiStringProperty("Shader", shaderFileName);
				}
				ImGui::Separator();

				std::vector<const char*> activeShaderFeatures;
				for (const auto& feature : pMaterialComponent->GetShaderFeatures())
				{
					activeShaderFeatures.emplace_back(nameof::nameof_enum(feature).data());
				}

				if (!activeShaderFeatures.empty())
				{
					if (ImGui::BeginCombo("##shaderFeatureCombo", "Active shader features"))
					{
						for (size_t index = 0; index < activeShaderFeatures.size(); ++index)
						{
							ImGui::Selectable(activeShaderFeatures[index], false);
						}
						ImGui::EndCombo();
					}
				}
			}

			ImGui::Separator();
			ImGui::PopStyleVar();
		}
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::CameraComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pCameraComponent = pSceneWorld->GetCameraComponent(entity);
	if (!pCameraComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("Camera Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		ImGui::BeginChild("Camera Component  Child Window");

		if (ImGuiUtils::ImGuiFloatProperty("Aspect", pCameraComponent->GetAspect()) ||
			ImGuiUtils::ImGuiFloatProperty("Field Of View", pCameraComponent->GetFov()) ||
			ImGuiUtils::ImGuiFloatProperty("NearPlane", pCameraComponent->GetNearPlane()) ||
			ImGuiUtils::ImGuiFloatProperty("FarPlane", pCameraComponent->GetFarPlane()))
		{
			pCameraComponent->Dirty();
			pCameraComponent->BuildProjectMatrix();
		}

		ImGuiUtils::ImGuiBoolProperty("Constrain Aspect Ratio", pCameraComponent->GetDoConstrainAspectRatio());

		ImGuiUtils::ImGuiFloatProperty("Exposure", pCameraComponent->GetExposure(), cd::Unit::None, 0.0f, 30.0f, false, 0.01f);
		ImGuiUtils::ImGuiEnumProperty("Tone Mapping Mode", pCameraComponent->GetToneMappingMode());
		ImGuiUtils::ImGuiFloatProperty("Gamma Correction", pCameraComponent->GetGammaCorrection(), cd::Unit::None, 0.0f, 1.0f);
		if (ImGui::TreeNode("Bloom"))
		{
			ImGuiUtils::ImGuiBoolProperty("Open Bloom", pCameraComponent->GetIsBloomEnable());
			if (pCameraComponent->GetIsBloomEnable())
			{
				ImGuiUtils::ImGuiIntProperty("DownSample Times", pCameraComponent->GetBloomDownSampleTimes(), cd::Unit::None, 4, 8, false, 1.0f);
				ImGuiUtils::ImGuiFloatProperty("Bloom Intensity", pCameraComponent->GetBloomIntensity(), cd::Unit::None, 0.0f, 3.0f, false, 0.01f);
				ImGuiUtils::ImGuiFloatProperty("Luminance Threshold", pCameraComponent->GetLuminanceThreshold(), cd::Unit::None, 0.0f, 3.0f, false, 0.01f);

				if (ImGui::TreeNode("Gaussian Blur"))
				{
					ImGuiUtils::ImGuiBoolProperty("Open Blur", pCameraComponent->GetIsBlurEnable());
					if (pCameraComponent->GetIsBlurEnable())
					{
						ImGuiUtils::ImGuiIntProperty("Blur Iteration", pCameraComponent->GetBlurTimes(), cd::Unit::None, 0, pCameraComponent->GetBlurMaxTimes(), false, 1.0f);
						ImGuiUtils::ImGuiFloatProperty("Blur Size", pCameraComponent->GetBlurSize(), cd::Unit::None, 0.0f, 3.0f);
						ImGuiUtils::ImGuiIntProperty("Blur Scaling", pCameraComponent->GetBlurScaling(), cd::Unit::None, 1, 4, false, 1.0f);
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		ImGui::EndChild();
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::LightComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pLightComponent = pSceneWorld->GetLightComponent(entity);
	if (!pLightComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("Light Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		cd::LightType lightType = pLightComponent->GetType();
		std::string lightTypeName(nameof::nameof_enum(lightType));

		ImGuiUtils::ImGuiStringProperty("Type", lightTypeName);
		ImGuiUtils::ImGuiBoolProperty("Cast Shadow", pLightComponent->IsCastShadow());
		ImGuiUtils::ColorPickerProperty("Color", pLightComponent->GetColor());

		float s_spotInnerAngle = 8.0f;
		float s_spotOuterAngle = 16.0f;
		bool spotInnerDirty = false;
		bool spotOuterDirty = false;

		if (cd::LightType::Point == lightType)
		{
			ImGuiUtils::ImGuiFloatProperty("Intensity", pLightComponent->GetIntensity(), cd::Unit::Lumen, 0.0f, 10000.0f, false, 5.0f);
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition(), cd::Unit::CenterMeter);
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange(), cd::Unit::CenterMeter, 0.0f, 10000.0f, false, 1.0f);
		}
		else if (cd::LightType::Directional == lightType)
		{
			ImGuiUtils::ImGuiFloatProperty("Intensity", pLightComponent->GetIntensity(), cd::Unit::Lux, 0.0f, 100.0f, false, 0.1f);
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection(), cd::Unit::Degree, cd::Vec3f(-1.0f), cd::Vec3f::One(), true);
		}
		else if (cd::LightType::Spot == lightType)
		{
			ImGuiUtils::ImGuiFloatProperty("Intensity", pLightComponent->GetIntensity(), cd::Unit::Lumen, 0.0f, 10000.0f, false, 5.0f);
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition(), cd::Unit::CenterMeter);
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection(), cd::Unit::Degree, cd::Vec3f(-1.0f), cd::Vec3f::One(), true);
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange(), cd::Unit::CenterMeter, 0.0f, 10000.0f, false, 1.0f);

			cd::Vec2f innerAndOuter = pLightComponent->GetInnerAndOuter();
			s_spotInnerAngle = innerAndOuter.x();
			s_spotOuterAngle = innerAndOuter.y();

			spotInnerDirty = ImGuiUtils::ImGuiFloatProperty("Inner Angle", s_spotInnerAngle, cd::Unit::Degree, 0.1f, 90.0f);
			spotOuterDirty = ImGuiUtils::ImGuiFloatProperty("Outer Angle", s_spotOuterAngle, cd::Unit::Degree, 0.1f, 90.0f);
			if (spotInnerDirty || spotOuterDirty)
			{
				pLightComponent->SetInnerAndOuter(s_spotInnerAngle, s_spotOuterAngle);
			}
		}
		else if (cd::LightType::Disk == lightType)
		{
			ImGuiUtils::ImGuiFloatProperty("Intensity", pLightComponent->GetIntensity(), cd::Unit::Lumen, 0.0f, 10000.0f, false, 5.0f);
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition(), cd::Unit::CenterMeter);
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection(), cd::Unit::Degree, cd::Vec3f(-1.0f), cd::Vec3f::One(), true);
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange(), cd::Unit::CenterMeter, 0.0f);
			ImGuiUtils::ImGuiFloatProperty("Radius", pLightComponent->GetRadius());
		}
		else if (cd::LightType::Rectangle == lightType)
		{
			ImGuiUtils::ImGuiFloatProperty("Intensity", pLightComponent->GetIntensity(), cd::Unit::Lumen, 0.0f, 10000.0f, false, 5.0f);
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiVectorProperty("Up", pLightComponent->GetUp());
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange());
			ImGuiUtils::ImGuiFloatProperty("Width", pLightComponent->GetWidth());
			ImGuiUtils::ImGuiFloatProperty("Height", pLightComponent->GetHeight());
		}
		else if (cd::LightType::Sphere == lightType)
		{
			ImGuiUtils::ImGuiFloatProperty("Intensity", pLightComponent->GetIntensity(), cd::Unit::Lumen, 0.0f, 10000.0f, false, 5.0f);
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiFloatProperty("Radius", pLightComponent->GetRadius());
		}
		else if (cd::LightType::Tube == lightType)
		{
			ImGuiUtils::ImGuiFloatProperty("Intensity", pLightComponent->GetIntensity(), cd::Unit::Lumen, 0.0f, 10000.0f, false, 5.0f);
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange());
			ImGuiUtils::ImGuiFloatProperty("Width", pLightComponent->GetWidth());
		}
		else
		{
			CD_ERROR("Unknown light type in inspector!");
		}
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::TerrainComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pTerrainComponent = pSceneWorld->GetTerrainComponent(entity);
	if (!pTerrainComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("Terrain Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{

		/*// Parameters
		ImGuiUtils::ImGuiVectorProperty("AlbedoColor", pMaterialComponent->GetAlbedoColor(), cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One());
		ImGuiUtils::ImGuiFloatProperty("MetallicFactor", pMaterialComponent->GetMetallicFactor(), cd::Unit::None, 0.0f, 1.0f);
		ImGuiUtils::ImGuiFloatProperty("RoughnessFactor", pMaterialComponent->GetRoughnessFactor(), cd::Unit::None, 0.0f, 1.0f);
		ImGuiUtils::ImGuiVectorProperty("EmissiveColor", pMaterialComponent->GetEmissiveColor(), cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One());
		ImGuiUtils::ImGuiBoolProperty("TwoSided", pMaterialComponent->GetTwoSided());
		ImGuiUtils::ImGuiStringProperty("BlendMode", cd::GetBlendModeName(pMaterialComponent->GetBlendMode()));
		if (cd::BlendMode::Mask == pMaterialComponent->GetBlendMode())
		{
			ImGuiUtils::ImGuiFloatProperty("AlphaCutOff", pMaterialComponent->GetAlphaCutOff(), cd::Unit::None, 0.0f, 1.0f);
		}*/
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

#ifdef ENABLE_DDGI
template<>
void UpdateComponentWidget<engine::DDGIComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pDDGIComponent = pSceneWorld->GetDDGIComponent(entity);
	if (!pDDGIComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("DDGI Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		ImGuiUtils::ImGuiVectorProperty("Origin", pDDGIComponent->GetVolumeOrigin(), cd::Unit::CenterMeter);
		ImGuiUtils::ImGuiVectorProperty("Probe Spacing", pDDGIComponent->GetProbeSpacing(), cd::Unit::CenterMeter);
		ImGuiUtils::ImGuiVectorProperty("Probe Count", pDDGIComponent->GetProbeCount(), cd::Unit::CenterMeter);
		ImGui::Separator();
		ImGuiUtils::ImGuiFloatProperty("Normal Bias", pDDGIComponent->GetNormalBias(), cd::Unit::None, 0.0f, 1.0f, false, 0.01f);
		ImGuiUtils::ImGuiFloatProperty("View Bias", pDDGIComponent->GetViewBias(), cd::Unit::None, 0.0f, 1.0f, false, 0.01f);
		ImGui::Separator();
		ImGuiUtils::ImGuiFloatProperty("Ambient Multiplier", pDDGIComponent->GetAmbientMultiplier(), cd::Unit::None, 0.0f, 10.0f);
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}
#endif

template<>
void UpdateComponentWidget<engine::SkyComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pSkyComponent = pSceneWorld->GetSkyComponent(entity);
	if (!pSkyComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("Sky Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		auto currentSkyType = pSkyComponent->GetSkyType();
		if (ImGuiUtils::ImGuiEnumProperty("SkyType", currentSkyType))
		{
			pSkyComponent->SetSkyType(currentSkyType);

			for (engine::Entity entity : pSceneWorld->GetMaterialEntities())
			{
				engine::MaterialComponent* pMaterialComponent = pSceneWorld->GetMaterialComponent(entity);
				if (!pMaterialComponent)
				{
					continue;
				}

				if (engine::SkyType::None == currentSkyType)
				{
					pMaterialComponent->DeactivateShaderFeature(engine::GetSkyTypeShaderFeature(engine::SkyType::AtmosphericScattering));
					pMaterialComponent->DeactivateShaderFeature(engine::GetSkyTypeShaderFeature(engine::SkyType::SkyBox));
				}
				else
				{
					pMaterialComponent->ActivateShaderFeature(engine::GetSkyTypeShaderFeature(currentSkyType));
				}
			}
		}

		if (pSkyComponent->GetAtmophericScatteringEnable())
		{
			ImGui::Separator();
			ImGuiUtils::ImGuiFloatProperty("Height Offset", pSkyComponent->GetHeightOffset(), cd::Unit::Kilometer, -1000.0f, 1000.0f, false, 0.1f);
			ImGuiUtils::ImGuiFloatProperty("Shadow Length", pSkyComponent->GetShadowLength(), cd::Unit::Kilometer, 0.0f, 10.0f, false, 0.1f);
			ImGuiUtils::ImGuiVectorProperty("Sun Direction", pSkyComponent->GetSunDirection(), cd::Unit::None, cd::Direction(-1.0f, -1.0f, -1.0f), cd::Direction::One(), true, 0.01f);
		}
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::ParticleEmitterComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pParticleEmitterComponent = pSceneWorld->GetParticleEmitterComponent(entity);
	if (!pParticleEmitterComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("Particle Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		ImGuiUtils::ImGuiEnumProperty("Render  Mode", pParticleEmitterComponent->GetRenderMode());
		ImGuiUtils::ImGuiEnumProperty("Particle Type", pParticleEmitterComponent->GetEmitterParticleType());
		ImGuiUtils::ImGuiVectorProperty("Emitter Range", pParticleEmitterComponent->GetEmitterShapeRange());
		ImGuiUtils::ImGuiIntProperty("Max Count", pParticleEmitterComponent->GetSpawnCount(), cd::Unit::None, 1, 300);
		ImGuiUtils::ImGuiVectorProperty("Velocity", pParticleEmitterComponent->GetEmitterVelocity());
		ImGuiUtils::ImGuiVectorProperty("Acceleration", pParticleEmitterComponent->GetEmitterAcceleration());
		ImGuiUtils::ColorPickerProperty("Color", pParticleEmitterComponent->GetEmitterColor());
		ImGuiUtils::ImGuiFloatProperty("LifeTime", pParticleEmitterComponent->GetLifeTime());
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

template<>
void UpdateComponentWidget<engine::ParticleForceFieldComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pParticleForceFieldComponent = pSceneWorld->GetParticleForceFieldComponent(entity);
	if (!pParticleForceFieldComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("ParticleForceField Component", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		ImGuiUtils::ImGuiVectorProperty("ForceFieldRange", pParticleForceFieldComponent->GetForceFieldRange());
		ImGuiUtils::ImGuiBoolProperty("RotationForceValue", pParticleForceFieldComponent->GetRotationForce());
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

}

namespace editor
{

Inspector::~Inspector()
{

}

void Inspector::Init()
{

}

void Inspector::Update()
{
	engine::RenderContext* pRenderContext = GetRenderContext();
	engine::SceneWorld* pSceneWorld = GetSceneWorld();
	if (engine::Entity selectedEntity = pSceneWorld->GetSelectedEntity(); selectedEntity != engine::INVALID_ENTITY)
	{
		// Entity will be invalid in two cases : 1.Select nothing 2.The selected entity has been deleted
		// Here we only want to capture the case 1 not to clear Inspector properties.
		// For case 2, it still uses a valid entity to update but nothing updated.
		// It is OK if we don't reuse the entity id intermediately.
		m_lastSelectedEntity = selectedEntity;
	}

	constexpr auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin(GetName(), &m_isEnable, flags);
	if (m_lastSelectedEntity == engine::INVALID_ENTITY)
	{
		// Call ImGui::Begin to show the panel though we will do nothing.
		ImGui::End();
		return;
	}

	ImGui::BeginChild("Inspector");
	details::UpdateComponentWidget<engine::NameComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::TransformComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::CameraComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::LightComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::SkyComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::TerrainComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::StaticMeshComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::MaterialComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::ParticleEmitterComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::ParticleForceFieldComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::CollisionMeshComponent>(pSceneWorld, m_lastSelectedEntity);
	details::UpdateComponentWidget<engine::BlendShapeComponent>(pSceneWorld, m_lastSelectedEntity);

#ifdef ENABLE_DDGI
	details::UpdateComponentWidget<engine::DDGIComponent>(pSceneWorld, m_lastSelectedEntity);
#endif

	ImGui::EndChild();

	ImGui::End();
}

}
