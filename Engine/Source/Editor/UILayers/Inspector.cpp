#include "Inspector.h"

#include "Graphics/GraphicsBackend.h"
#include "ImGui/ImGuiUtils.hpp"
#include "Path/Path.h"

namespace details
{

CD_FORCEINLINE bool EnableAtmosphericScattering()
{
	return engine::GraphicsBackend::OpenGL != engine::Path::GetGraphicsBackend() &&
		engine::GraphicsBackend::Vulkan != engine::Path::GetGraphicsBackend();
}

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
		ImGuiUtils::ImGuiStringProperty("Name", pMaterialComponent->GetName());
		
		// Parameters
		ImGuiUtils::ImGuiVectorProperty("AlbedoColor", pMaterialComponent->GetAlbedoColor(), cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One());
		ImGuiUtils::ImGuiFloatProperty("MetallicFactor", pMaterialComponent->GetMetallicFactor(), cd::Unit::None, 0.0f, 1.0f);
		ImGuiUtils::ImGuiFloatProperty("RoughnessFactor", pMaterialComponent->GetRoughnessFactor(), cd::Unit::None, 0.0f, 1.0f);
		ImGuiUtils::ImGuiVectorProperty("EmissiveColor", pMaterialComponent->GetEmissiveColor(), cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One());
		ImGuiUtils::ImGuiBoolProperty("TwoSided", pMaterialComponent->GetTwoSided());
		ImGuiUtils::ImGuiStringProperty("BlendMode", cd::GetBlendModeName(pMaterialComponent->GetBlendMode()));
		if (cd::BlendMode::Mask == pMaterialComponent->GetBlendMode())
		{
			ImGuiUtils::ImGuiFloatProperty("AlphaCutOff", pMaterialComponent->GetAlphaCutOff(), cd::Unit::None, 0.0f, 1.0f);
		}

		// Textures
		for (int textureTypeValue = 0; textureTypeValue < static_cast<int>(cd::MaterialTextureType::Count); ++textureTypeValue)
		{
			if (engine::MaterialComponent::TextureInfo* textureInfo = pMaterialComponent->GetTextureInfo(static_cast<cd::MaterialPropertyGroup>(textureTypeValue)))
			{
				const char* title = cd::GetMaterialPropertyGroupName(static_cast<cd::MaterialPropertyGroup>(textureTypeValue));
				bool isOpen = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
				ImGui::Separator();

				std::string uvOffset = std::string(title) + std::string(" UVOffset");
				std::string uvScale = std::string(title) + std::string(" UVScale");
				if (isOpen)
				{
					ImGuiUtils::ImGuiVectorProperty(uvOffset.c_str(), textureInfo->GetUVOffset());
					ImGuiUtils::ImGuiVectorProperty(uvScale.c_str(), textureInfo->GetUVScale());

				}

				ImGui::Separator();
				ImGui::PopStyleVar();
			}
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
		if (ImGuiUtils::ImGuiFloatProperty("Aspect", pCameraComponent->GetAspect()) ||
			ImGuiUtils::ImGuiFloatProperty("Field Of View", pCameraComponent->GetFov()) ||
			ImGuiUtils::ImGuiFloatProperty("NearPlane", pCameraComponent->GetNearPlane()) ||
			ImGuiUtils::ImGuiFloatProperty("FarPlane", pCameraComponent->GetFarPlane()))
		{
			pCameraComponent->Dirty();
			pCameraComponent->BuildProjectMatrix();
		}

		ImGuiUtils::ImGuiBoolProperty("Constrain Aspect Ratio", pCameraComponent->GetDoConstrainAspectRatio());
		ImGuiUtils::ImGuiBoolProperty("Post Processing", pCameraComponent->GetIsPostProcessEnable());
		ImGuiUtils::ImGuiVectorProperty("Gamma Correction", pCameraComponent->GetGammaCorrection(), cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One());
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
		ImGuiUtils::ImGuiVectorProperty("Color", pLightComponent->GetColor(), cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One());

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
void UpdateComponentWidget<engine::DDGIComponent>(engine::SceneWorld *pSceneWorld, engine::Entity entity)
{
	auto *pDDGIComponent = pSceneWorld->GetDDGIComponent(entity);
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
		ImGuiUtils::ImGuiFloatProperty("Ambient Multiplier", pDDGIComponent->GetAmbientMultiplier(), cd::Unit::None, 0.0f, 10.0f);
	}

	ImGui::Separator();
	ImGui::PopStyleVar();
}

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
			if (!EnableAtmosphericScattering() && static_cast<engine::SkyType>(type) == engine::SkyType::AtmosphericScattering)
			{
				continue;
			}
			skyTypes.emplace_back(GetSkyTypeName(static_cast<engine::SkyType>(type)));
		}

		static const char* crtItem = GetSkyTypeName(engine::SkyType::SkyBox);
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

	details::UpdateComponentWidget<engine::NameComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::TransformComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::StaticMeshComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::MaterialComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::CameraComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::LightComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::DDGIComponent>(pSceneWorld, selectedEntity);
	details::UpdateComponentWidget<engine::SkyComponent>(pSceneWorld, selectedEntity);

	ImGui::End();
}

}
