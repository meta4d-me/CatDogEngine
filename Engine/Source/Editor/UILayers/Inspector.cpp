#include "Inspector.h"

#include "Rendering/RenderContext.h"
#include "Graphics/GraphicsBackend.h"
#include "ImGui/ImGuiUtils.hpp"
#include "Path/Path.h"

namespace details
{

template<typename Component>
void UpdateComponentWidget(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
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
		ImGuiUtils::ColorPickerProperty("AlbedoColor", pMaterialComponent->GetAlbedoColor());
		ImGuiUtils::ImGuiFloatProperty("MetallicFactor", pMaterialComponent->GetMetallicFactor(), cd::Unit::None, 0.0f, 1.0f);
		ImGuiUtils::ImGuiFloatProperty("RoughnessFactor", pMaterialComponent->GetRoughnessFactor(), cd::Unit::None, 0.0f, 1.0f);
		ImGuiUtils::ColorPickerProperty("EmissiveColor", pMaterialComponent->GetEmissiveColor());
		ImGuiUtils::ImGuiBoolProperty("TwoSided", pMaterialComponent->GetTwoSided());
		ImGuiUtils::ImGuiStringProperty("BlendMode", nameof::nameof_enum(pMaterialComponent->GetBlendMode()).data());
		if (cd::BlendMode::Mask == pMaterialComponent->GetBlendMode())
		{
			ImGuiUtils::ImGuiFloatProperty("AlphaCutOff", pMaterialComponent->GetAlphaCutOff(), cd::Unit::None, 0.0f, 1.0f);
		}

		// Textures
		for (int textureTypeValue = 0; textureTypeValue < static_cast<int>(cd::MaterialTextureType::Count); ++textureTypeValue)
		{
			if (engine::MaterialComponent::TextureInfo* pTextureInfo = pMaterialComponent->GetTextureInfo(static_cast<cd::MaterialPropertyGroup>(textureTypeValue)))
			{
				const char* title = nameof::nameof_enum(static_cast<cd::MaterialPropertyGroup>(textureTypeValue)).data();
				bool isOpen = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
				ImGui::Separator();

				std::string uvOffset = std::string(title) + std::string(" UVOffset");
				std::string uvScale = std::string(title) + std::string(" UVScale");
				if (isOpen)
				{
					ImGuiUtils::ImGuiVectorProperty(uvOffset.c_str(), pTextureInfo->GetUVOffset());
					ImGuiUtils::ImGuiVectorProperty(uvScale.c_str(), pTextureInfo->GetUVScale());
				}

				ImGui::Separator();
				ImGui::PopStyleVar();
			}
		}

		// Shaders
		const char* title = "Shader";
		bool isOpen = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Separator();

		if (isOpen)
		{
			ImGuiUtils::ImGuiStringProperty("Vertex Shader", pMaterialComponent->GetVertexShaderName());
			ImGuiUtils::ImGuiStringProperty("Fragment Shader", pMaterialComponent->GetFragmentShaderName());
			ImGui::Separator();

			std::vector<const char*> activeUberOptions;
			for (const auto& uber : pMaterialComponent->GetUberShaderOptions())
			{
				activeUberOptions.emplace_back(nameof::nameof_enum(uber).data());
			}

			if (!activeUberOptions.empty())
			{
				if (ImGui::BeginCombo("##combo", "Active uber options"))
				{
					for (size_t index = 0; index < activeUberOptions.size(); ++index)
					{
						ImGui::Selectable(activeUberOptions[index], false);
					}
					ImGui::EndCombo();
				}
			}
		}

		ImGui::Separator();
		ImGui::PopStyleVar();
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

		if (ImGui::TreeNode("Post Processing"))
		{

			ImGuiUtils::ImGuiBoolProperty("Tone Mapping", pCameraComponent->GetIsToneMapping());
			ImGuiUtils::ImGuiFloatProperty("Gamma Correction", pCameraComponent->GetGammaCorrection(), cd::Unit::None, 0.0f, 1.0f);
			if (ImGui::TreeNode("Bloom"))
			{
				ImGuiUtils::ImGuiBoolProperty("Open Bloom", pCameraComponent->GetIsBloomEnable());
				if (pCameraComponent->GetIsBloomEnable())
				{
					ImGuiUtils::ImGuiIntProperty("DownSampleTimes", pCameraComponent->GetBloomDownSampleTimes(), cd::Unit::None, 4, 8, false, 1.0f);
					ImGuiUtils::ImGuiFloatProperty("Bloom Intensity", pCameraComponent->GetBloomIntensity(), cd::Unit::None, 0.0f, 3.0f, false, 0.01f);
					ImGuiUtils::ImGuiFloatProperty("Luminance Threshold", pCameraComponent->GetLuminanceThreshold(), cd::Unit::None, 0.0f, 3.0f, false, 0.01f);
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Gaussian Blur"))
			{
				ImGuiUtils::ImGuiBoolProperty("Open Blur", pCameraComponent->GetIsBlurEnable());
				if (pCameraComponent->GetIsBlurEnable())
				{
					ImGuiUtils::ImGuiIntProperty("BlurIteration", pCameraComponent->GetBlurTimes(), cd::Unit::None, 0, 20, false, 1.0f);
					ImGuiUtils::ImGuiFloatProperty("Blur Size", pCameraComponent->GetBlurSize(), cd::Unit::None, 0.0f, 3.0f);
					ImGuiUtils::ImGuiIntProperty("Blur Scaling", pCameraComponent->GetBlurScaling(), cd::Unit::None, 1, 4, false, 1.0f);
				}
				ImGui::TreePop();
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
		std::string lightTypeName = cd::GetLightTypeName(lightType);

		ImGuiUtils::ImGuiStringProperty("Type", lightTypeName);
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

			spotInnerDirty = ImGuiUtils::ImGuiFloatProperty("InnerAngle", s_spotInnerAngle, cd::Unit::Degree, 0.1f, 90.0f);
			spotOuterDirty = ImGuiUtils::ImGuiFloatProperty("OuterAngle", s_spotOuterAngle, cd::Unit::Degree, 0.1f, 90.0f);
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
		std::vector<const char*> skyTypes;
		for (size_t type = 0; type < static_cast<size_t>(engine::SkyType::Count); ++type)
		{
			if (!pSkyComponent->GetAtmophericScatteringEnable() && engine::SkyType::AtmosphericScattering == static_cast<engine::SkyType>(type))
			{
				continue;
			}
			skyTypes.emplace_back(nameof::nameof_enum(static_cast<engine::SkyType>(type)).data());
		}

		if (!skyTypes.empty())
		{
			static const char* crtItem = nameof::nameof_enum(engine::SkyType::SkyBox).data();
			if (ImGui::BeginCombo("##combo", crtItem))
			{
				for (size_t index = 0; index < skyTypes.size(); ++index)
				{
					bool isSelected = (crtItem == skyTypes[index]);
					if (ImGui::Selectable(skyTypes[index], isSelected))
					{
						crtItem = skyTypes[index];
						pSkyComponent->SetSkyType(static_cast<engine::SkyType>(index));
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
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
void UpdateComponentWidget<engine::ParticleComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pParticleComponent = pSceneWorld->GetParticleComponent(entity);
	if (!pParticleComponent)
	{
		return;
	}

	bool isOpen = ImGui::CollapsingHeader("ParticleComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{

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
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin(GetName(), &m_isEnable, flags);

	engine::SceneWorld* pSceneWorld = GetSceneWorld();
	engine::Entity selectedEntity = pSceneWorld->GetSelectedEntity();
	if (engine::INVALID_ENTITY == selectedEntity)
	{
		ImGui::End();
		return;
	}

	ImGui::BeginChild("Inspector");

	details::UpdateComponentWidget<engine::NameComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::TransformComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::StaticMeshComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::MaterialComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::CameraComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::LightComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::SkyComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::TerrainComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::ParticleComponent>(pSceneWorld, selectedEntity);

#ifdef ENABLE_DDGI
	details::UpdateComponentWidget<engine::DDGIComponent>(pSceneWorld, selectedEntity);
#endif

	ImGui::EndChild();

	ImGui::End();
}

}
