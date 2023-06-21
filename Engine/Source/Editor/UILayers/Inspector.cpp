#include "Inspector.h"

#include "UILayers/ImGuiUtils.hpp"


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
		ImGuiUtils::ImGuiProperty<std::string>("Name", pNameComponent->GetNameForWrite());
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
		if (ImGuiUtils::ImGuiProperty<cd::Transform>("Transform", pTransformComponent->GetTransform()))
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
		ImGuiUtils::ImGuiProperty<cd::Vec3f>("AlbedoColor", pMaterialComponent->GetAlbedoColor(), cd::Vec3f::Zero(), cd::Vec3f::One());
		ImGuiUtils::ImGuiProperty<cd::Vec3f>("EmissiveColor", pMaterialComponent->GetEmissiveColor(), cd::Vec3f::Zero(), cd::Vec3f::One());
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
		if (ImGuiUtils::ImGuiProperty<float>("Aspect", pCameraComponent->GetAspect()) ||
			ImGuiUtils::ImGuiProperty<float>("Field Of View", pCameraComponent->GetFov()) ||
			ImGuiUtils::ImGuiProperty<float>("NearPlane", pCameraComponent->GetNearPlane()) ||
			ImGuiUtils::ImGuiProperty<float>("FarPlane", pCameraComponent->GetFarPlane()))
		{
			pCameraComponent->Dirty();
			pCameraComponent->Build();
		}

		ImGuiUtils::ImGuiProperty<bool>("Constrain Aspect Ratio", pCameraComponent->GetDoConstrainAspectRatio());
		ImGuiUtils::ImGuiProperty<bool>("Post Processing", pCameraComponent->GetIsPostProcessEnable());
		ImGuiUtils::ImGuiProperty<cd::Vec3f>("Gamma Correction", pCameraComponent->GetGammaCorrection(), cd::Vec3f::Zero(), cd::Vec3f::One());
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

		ImGuiUtils::ImGuiProperty<std::string>("Type", lightTypeName);
		ImGuiUtils::ImGuiProperty<cd::Vec3f>("Color", pLightComponent->GetColor());
		ImGuiUtils::ImGuiProperty<float>("Intensity", pLightComponent->GetIntensity());

		static float s_spotInnerAngle = 8.0f;
		static float s_spotOuterAngle = 16.0f;
		bool spotInnerDirty = false;
		bool spotOuterDirty = false;

		switch (lightType)
		{
		case cd::LightType::Point:
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiProperty<float>("Range", pLightComponent->GetRange());
			break;
		case cd::LightType::Directional:
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			break;
		case cd::LightType::Spot:
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiProperty<float>("Range", pLightComponent->GetRange());

			cd::Vec2f innerAndOuter = pLightComponent->GetInnerAndOuter();
			s_spotInnerAngle = innerAndOuter.x();
			s_spotOuterAngle = innerAndOuter.y();

			spotInnerDirty = ImGuiUtils::ImGuiProperty<float>("InnerAngle", s_spotInnerAngle);
			spotOuterDirty = ImGuiUtils::ImGuiProperty<float>("OuterAngle", s_spotOuterAngle);
			if(spotInnerDirty || spotOuterDirty)
			{
				pLightComponent->SetInnerAndOuter(s_spotInnerAngle, s_spotOuterAngle);
			}
			
			break;
		case cd::LightType::Disk:
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiProperty<float>("Range", pLightComponent->GetRange());
			ImGuiUtils::ImGuiProperty<float>("Radius", pLightComponent->GetRadius());
			break;
		case cd::LightType::Rectangle:
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Up", pLightComponent->GetUp());
			ImGuiUtils::ImGuiProperty<float>("Range", pLightComponent->GetRange());
			ImGuiUtils::ImGuiProperty<float>("Width", pLightComponent->GetWidth());
			ImGuiUtils::ImGuiProperty<float>("Height", pLightComponent->GetHeight());
			break;
		case cd::LightType::Sphere:
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiProperty<float>("Radius", pLightComponent->GetRadius());
			break;
		case cd::LightType::Tube:
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiUtils::ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiUtils::ImGuiProperty<float>("Range", pLightComponent->GetRange());
			ImGuiUtils::ImGuiProperty<float>("Width", pLightComponent->GetWidth());
			break;
		default:
			assert("TODO");
			break;
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

	ImGui::End();
}

}
