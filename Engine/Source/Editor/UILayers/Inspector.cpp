#include "Inspector.h"

#include "ImGui/ImGuiUtils.hpp"


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

	bool isHeaderOpen = ImGui::CollapsingHeader("NameComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
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

	bool isHeaderOpen = ImGui::CollapsingHeader("TransformComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
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

	bool isOpen = ImGui::CollapsingHeader("MaterialComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isOpen)
	{
		ImGuiUtils::ImGuiVectorProperty("AlbedoColor", pMaterialComponent->GetAlbedoColor(), cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One());
		ImGuiUtils::ImGuiVectorProperty("EmissiveColor", pMaterialComponent->GetEmissiveColor(), cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One());
		ImGuiUtils::ImGuiBoolProperty("TwoSided", pMaterialComponent->GetTwoSided());
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

	bool isOpen = ImGui::CollapsingHeader("CameraComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
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
			pCameraComponent->Build();
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

	bool isOpen = ImGui::CollapsingHeader("LightComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();
	
	if (isOpen)
	{
		cd::LightType lightType = pLightComponent->GetType();
		std::string lightTypeName = cd::GetLightTypeName(lightType);

		ImGuiUtils::ImGuiStringProperty("Type", lightTypeName);
		ImGuiUtils::ImGuiVectorProperty("Color", pLightComponent->GetColor(), cd::Unit::None, cd::Vec3f::Zero(), cd::Vec3f::One());
		ImGuiUtils::ImGuiFloatProperty("Intensity", pLightComponent->GetIntensity(), cd::Unit::Lumen, 0.0f, 10000.0f, false, 10.0f);

		float s_spotInnerAngle = 8.0f;
		float s_spotOuterAngle = 16.0f;
		bool spotInnerDirty = false;
		bool spotOuterDirty = false;

		switch (lightType)
		{
		case cd::LightType::Point:
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition(), cd::Unit::CenterMeter);
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange(), cd::Unit::CenterMeter, 0.0f, 10000.0f, false, 1.0f);
			break;
		case cd::LightType::Directional:
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection(), cd::Unit::Degree, cd::Vec3f(-1.0f), cd::Vec3f::One(), true);
			break;
		case cd::LightType::Spot:
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition(), cd::Unit::CenterMeter);
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection(), cd::Unit::Degree, cd::Vec3f(-1.0f), cd::Vec3f::One(), true);
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange(), cd::Unit::CenterMeter, 0.0f, 10000.0f, false, 1.0f);

			cd::Vec2f innerAndOuter = pLightComponent->GetInnerAndOuter();
			s_spotInnerAngle = innerAndOuter.x();
			s_spotOuterAngle = innerAndOuter.y();

			spotInnerDirty = ImGuiUtils::ImGuiFloatProperty("InnerAngle", s_spotInnerAngle, cd::Unit::Degree, 0.1f, 90.0f);
			spotOuterDirty = ImGuiUtils::ImGuiFloatProperty("OuterAngle", s_spotOuterAngle, cd::Unit::Degree, 0.1f, 90.0f);
			if(spotInnerDirty || spotOuterDirty)
			{
				pLightComponent->SetInnerAndOuter(s_spotInnerAngle, s_spotOuterAngle);
			}
			
			break;
		case cd::LightType::Disk:
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition(), cd::Unit::CenterMeter);
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection(), cd::Unit::Degree, cd::Vec3f(-1.0f), cd::Vec3f::One(), true);
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange(), cd::Unit::CenterMeter, 0.0f);
			ImGuiUtils::ImGuiFloatProperty("Radius", pLightComponent->GetRadius());
			break;
		case cd::LightType::Rectangle:
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiVectorProperty("Up", pLightComponent->GetUp());
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange());
			ImGuiUtils::ImGuiFloatProperty("Width", pLightComponent->GetWidth());
			ImGuiUtils::ImGuiFloatProperty("Height", pLightComponent->GetHeight());
			break;
		case cd::LightType::Sphere:
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiFloatProperty("Radius", pLightComponent->GetRadius());
			break;
		case cd::LightType::Tube:
			ImGuiUtils::ImGuiVectorProperty("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiVectorProperty("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiFloatProperty("Range", pLightComponent->GetRange());
			ImGuiUtils::ImGuiFloatProperty("Width", pLightComponent->GetWidth());
			break;
		default:
			assert("TODO");
			break;
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

	bool isOpen = ImGui::CollapsingHeader("DDGIComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
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

	ImGui::End();
}

}
