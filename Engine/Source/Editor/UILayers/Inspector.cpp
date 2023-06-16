#include "Inspector.h"

#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "ImGui/ImGuiContextInstance.h"
#include "Rendering/RenderContext.h"
#include "Resources/ResourceBuilder.h"

#include <imgui/imgui.h>

namespace details
{

template<typename T>
bool ImGuiProperty(const char* pName, T& value, const T& minValue = {}, const T& maxValue = {})
{
	bool dirty = false;
	bool isUniform = true;

	if constexpr (std::is_same<T, std::string>())
	{
		ImGui::Columns(2);
		ImGui::TextUnformatted(pName);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		ImGui::TextUnformatted(value.c_str());

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::Columns(1);
	}
	else if constexpr (std::is_same<T, bool>())
	{
		if (ImGui::Checkbox(pName, &value))
		{
			dirty = true;
		}
	}
	else if constexpr (std::is_same<T, float>())
	{
		ImGui::Columns(2);
		ImGui::TextUnformatted(pName);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string labelName = std::format("##{}", pName);
		float delta = maxValue - minValue;
		float speed = cd::Math::IsEqualToZero(delta) ? 1.0f : delta * 0.05f;
		if (ImGui::DragFloat(labelName.c_str(), &value, speed, minValue, maxValue))
		{
			dirty = true;
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::Columns(1);
	}
	else if constexpr (std::is_same<T, cd::Vec2f>() || std::is_same<T, cd::Vec3f>() || std::is_same<T, cd::Vec4f>())
	{
		ImGui::Columns(2);
		ImGui::TextUnformatted(pName);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string labelName = std::format("##{}", pName);
		float delta = maxValue.x() - minValue.x();
		float speed = cd::Math::IsEqualToZero(delta) ? 1.0f : delta * 0.05f;
		if constexpr (std::is_same<T, cd::Vec2f>())
		{
			if (ImGui::DragFloat2(labelName.c_str(), value.Begin(), speed, minValue.x(), maxValue.x()))
			{
				dirty = true;
			}
		}
		else if constexpr (std::is_same<T, cd::Vec3f>())
		{
			if (ImGui::DragFloat3(labelName.c_str(), value.Begin(), speed, minValue.x(), maxValue.x()))
			{
				dirty = true;
			}
		}
		else if constexpr (std::is_same<T, cd::Vec4f>())
		{
			if (ImGui::DragFloat4(labelName.c_str(), value.Begin(), speed, minValue.x(), maxValue.x()))
			{
				dirty = true;
			}
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::Columns(1);
	}
	else if constexpr (std::is_same<T, cd::Transform>())
	{
		if (ImGuiProperty<cd::Vec3f>("Translation", value.GetTranslation()))
		{
			dirty = true;
		}

		cd::Vec3f eularAngles = value.GetRotation().ToEulerAngles();
		if (ImGuiProperty<cd::Vec3f>("Rotation", eularAngles, cd::Vec3f::Zero(), cd::Vec3f(360.0f)))
		{
			float pitch = std::min(eularAngles.x(), 89.9f);
			pitch = std::max(pitch, -89.9f);

			value.SetRotation(cd::Quaternion::FromPitchYawRoll(pitch, eularAngles.y(), eularAngles.z()));
			dirty = true;
		}

		cd::Vec3f originScale = value.GetScale();
		cd::Vec3f scale = originScale;
		ImGui::TextUnformatted("Scale");
		ImGui::SameLine();
		bool UniformScaleEnabled = engine::TransformComponent::DoUseUniformScale();
		ImGui::Checkbox("Uniform",&UniformScaleEnabled);
		engine::TransformComponent::SetUseUniformScale(UniformScaleEnabled);

		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		if (ImGui::DragFloat3("##Scale", scale.Begin(),0.1f,0.001f,999.0f))
		{
			if (!cd::Math::IsEqualTo(scale.x(), originScale.x()))
			{
				if (UniformScaleEnabled)
				{
					float ratio = scale.x() / originScale.x();
					cd::Vec3f _scale = value.GetScale();
					_scale *= ratio;
					value.SetScale(_scale);
					
					dirty = true;
				}
				else
				{
					value.SetScale(scale);
					dirty = true;
				}
			}

			if (!cd::Math::IsEqualTo(scale.y(), originScale.y()))
			{
				if (UniformScaleEnabled)
				{
					float ratio = scale.y() / originScale.y();
					cd::Vec3f _scale = value.GetScale();
					_scale *= ratio;
					value.SetScale(_scale);
					dirty = true;
				}
				else
				{
					value.SetScale(scale);
					dirty = true;
				}
			}

			if (!cd::Math::IsEqualTo(scale.z(), originScale.z()))
			{
				if (UniformScaleEnabled)
				{
					float ratio = scale.z() / originScale.z();
					cd::Vec3f _scale = value.GetScale();
					_scale *= ratio;
					value.SetScale(_scale);
					dirty = true;
				}
				else
				{
					value.SetScale(scale);
					dirty = true;
				}
			}
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::Columns(1);
	}
	else
	{
		static_assert("Unsupported data type for imgui property.");
	}

	return dirty;
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

	bool isHeaderOpen = ImGui::CollapsingHeader("NameComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Separator();

	if (isHeaderOpen)
	{
		ImGuiProperty<std::string>("Name", pNameComponent->GetNameForWrite());
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
		if (ImGuiProperty<cd::Transform>("Transform", pTransformComponent->GetTransform()))
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
		std::vector<std::string> fileNames;
		editor::Inspector* imguiBaseLayer = nullptr; 
		std::filesystem::path dirPath{ "C:/CatDogEngine/Projects/Test/test" };
		std::filesystem::path frontPath{ "test" }; 

		for (const auto& it : std::filesystem::directory_iterator(dirPath))
		{
			std::string fileName = it.path().filename().string();
			fileNames.push_back(fileName);
		}

		std::vector<std::string> textyrePaths;
		for (int i = 0;  i < fileNames.size();  ++i)
		{
			std::string fullpath = (frontPath / fileNames[i]).string(); 
			imguiBaseLayer->GetRenderContext()->CreateTexture(fullpath.c_str());
			textyrePaths.emplace_back(std::move(fullpath));
		}

		static int currentItem = 0;
		if (ImGui::BeginCombo("Select Texture", fileNames[currentItem].c_str()))
		{
			for (int i = 0;  i < fileNames.size();  ++i)
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

		bgfx::TextureHandle textureHandle = imguiBaseLayer->GetRenderContext()->GetTexture(engine::StringCrc(textyrePaths[currentItem].c_str()));
		ImGui::Separator();
		ImGui::Image(ImTextureID(textureHandle.idx), ImVec2(64, 64));

		ImGui::Separator();
		ImGuiProperty<cd::Vec3f>("AlbedoColor", pMaterialComponent->GetAlbedoColor(), cd::Vec3f::Zero(), cd::Vec3f::One());
		ImGuiProperty<cd::Vec3f>("EmissiveColor", pMaterialComponent->GetEmissiveColor(), cd::Vec3f::Zero(), cd::Vec3f::One());
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
		if (ImGuiProperty<float>("Aspect", pCameraComponent->GetAspect()) ||
			ImGuiProperty<float>("Field Of View", pCameraComponent->GetFov()) ||
			ImGuiProperty<float>("NearPlane", pCameraComponent->GetNearPlane()) ||
			ImGuiProperty<float>("FarPlane", pCameraComponent->GetFarPlane()))
		{
			pCameraComponent->Dirty();
			pCameraComponent->Build();
		}

		ImGuiProperty<bool>("Constrain Aspect Ratio", pCameraComponent->GetDoConstrainAspectRatio());
		ImGuiProperty<bool>("Post Processing", pCameraComponent->GetIsPostProcessEnable());
		ImGuiProperty<cd::Vec3f>("Gamma Correction", pCameraComponent->GetGammaCorrection(), cd::Vec3f::Zero(), cd::Vec3f::One());
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

		ImGuiProperty<std::string>("Type", lightTypeName);
		ImGuiProperty<cd::Vec3f>("Color", pLightComponent->GetColor());
		ImGuiProperty<float>("Intensity", pLightComponent->GetIntensity());

		switch (lightType)
		{
		case cd::LightType::Point:
			ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiProperty<float>("Range", pLightComponent->GetRange());
			break;
		case cd::LightType::Directional:
			ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			break;
		case cd::LightType::Spot:
			ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiProperty<float>("Range", pLightComponent->GetRange());
			ImGuiProperty<float>("AngleScale", pLightComponent->GetAngleScale());
			ImGuiProperty<float>("AngleOffset", pLightComponent->GetAngleOffset());
			break;
		case cd::LightType::Disk:
			ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiProperty<float>("Range", pLightComponent->GetRange());
			ImGuiProperty<float>("Radius", pLightComponent->GetRadius());
			break;
		case cd::LightType::Rectangle:
			ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiProperty<cd::Vec3f>("Up", pLightComponent->GetUp());
			ImGuiProperty<float>("Range", pLightComponent->GetRange());
			ImGuiProperty<float>("Width", pLightComponent->GetWidth());
			ImGuiProperty<float>("Height", pLightComponent->GetHeight());
			break;
		case cd::LightType::Sphere:
			ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiProperty<float>("Radius", pLightComponent->GetRadius());
			break;
		case cd::LightType::Tube:
			ImGuiProperty<cd::Vec3f>("Position", pLightComponent->GetPosition());
			ImGuiProperty<cd::Vec3f>("Direction", pLightComponent->GetDirection());
			ImGuiProperty<float>("Range", pLightComponent->GetRange());
			ImGuiProperty<float>("Width", pLightComponent->GetWidth());
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
