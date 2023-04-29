#include "Inspector.h"

#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "ImGui/ImGuiContextInstance.h"

#include <imgui/imgui.h>

namespace details
{

template<typename T>
bool ImGuiProperty(const char* pName, T& value)
{
	bool dirty = false;
	bool isUniform = true;

	if constexpr (std::is_same<T, std::string>())
	{
		ImGui::TextUnformatted(pName);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		ImGui::TextUnformatted(value.c_str());

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}
	else if constexpr (std::is_same<T, bool>())
	{
		ImGui::TextUnformatted(pName);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		if (ImGui::Checkbox(pName, &value))
		{
			dirty = true;
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}
	else if constexpr (std::is_same<T, float>())
	{
		ImGui::TextUnformatted(pName);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		if (ImGui::DragFloat(pName, &value))
		{
			dirty = true;
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();
	}
	else if constexpr (std::is_same<T, cd::Transform>())
	{
		ImGui::TextUnformatted("Translation");
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		if (ImGui::DragFloat3("##Translation", value.GetTranslation().Begin()))
		{
			dirty = true;
		}

		if (ImGui::IsItemActivated() && ImGui::GetIO().MouseDelta.x != 0)
		{

		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		ImGui::TextUnformatted("Rotation");
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		cd::Vec3f originEulerAngles = value.GetRotation().ToEulerAngles();
		cd::Vec3f eulerAngles = originEulerAngles;
		if (ImGui::DragFloat3("##Rotation", eulerAngles.Begin()))
		{
			if (!cd::Math::IsEqualTo(eulerAngles.x(), originEulerAngles.x()))
			{
				cd::Quaternion quaternion = cd::Quaternion::FromAxisAngle(cd::Vec3f(1.0f, 0.0f, 0.0f), cd::Math::DegreeToRadian(eulerAngles.x()));
				value.SetRotation(quaternion);
				dirty = true;
			}
			
			if (!cd::Math::IsEqualTo(eulerAngles.y(), originEulerAngles.y()))
			{
				cd::Quaternion quaternion = cd::Quaternion::FromAxisAngle(cd::Vec3f(0.0f, 1.0f, 0.0f), cd::Math::DegreeToRadian(eulerAngles.y()));
				value.SetRotation(quaternion);
				dirty = true;
			}
			
			if (!cd::Math::IsEqualTo(eulerAngles.z(), originEulerAngles.z()))
			{
				cd::Quaternion quaternion = cd::Quaternion::FromAxisAngle(cd::Vec3f(0.0f, 0.0f, 1.0f), cd::Math::DegreeToRadian(eulerAngles.z()));
				value.SetRotation(quaternion);
				dirty = true;
			}
		}


		ImGui::PopItemWidth();
		ImGui::NextColumn();

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
	ImGui::Columns(2);
	ImGui::Separator();

	if (isHeaderOpen)
	{
		ImGuiProperty<std::string>("Name", pNameComponent->GetNameForWrite());
	}

	ImGui::Columns(1);
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
	ImGui::Columns(2);
	ImGui::Separator();

	if (isHeaderOpen)
	{
		if (ImGuiProperty<cd::Transform>("Transform", pTransformComponent->GetTransform()))
		{
			pTransformComponent->Dirty();
			pTransformComponent->Build();
		}
	}

	ImGui::Columns(1);
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

}

template<>
void UpdateComponentWidget<engine::CameraComponent>(engine::SceneWorld* pSceneWorld, engine::Entity entity)
{
	auto* pCameraComponent = pSceneWorld->GetCameraComponent(entity);
	if (!pCameraComponent)
	{
		return;
	}
	bool isCamOpen = ImGui::CollapsingHeader("CameraComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(1);
	ImGui::Separator();
	if (isCamOpen)
	{
		bool isCamSetOpen = ImGui::CollapsingHeader("Camera Setting", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		if (isCamSetOpen)
		{
			ImGui::Columns(2);
			ImGui::TextUnformatted("Aspect");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			float aspect = pCameraComponent->GetAspect();
			if (ImGui::DragFloat("##Aspect", &aspect,0.005f, 0.1f,5.0f))
			{
				pCameraComponent->SetAspect(aspect);
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::TextUnformatted("Fov");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			float fov = pCameraComponent->GetFov();
			if (ImGui::DragFloat("##Fov", &fov,0.1f,5.0f, 170.f ))
			{
				pCameraComponent->SetFov(fov);
			}

		}

		ImGui::Columns(1);

		bool isCamOptOpen = ImGui::CollapsingHeader("Camera Option", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		if (isCamOptOpen)
		{
			ImGui::Columns(2);
			ImGui::TextUnformatted("Constrain Aspect Ratio");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			bool constrain = pCameraComponent->DoConstrainAspectRatio();
			ImGui::Checkbox(" ", &constrain);
			pCameraComponent->SetConstrainAspectRatio(constrain);
		}
	

	}
	ImGui::Columns(1);
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
	bool isLightOpen = ImGui::CollapsingHeader("LightComponent", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(1);
	ImGui::Separator();
	if (isLightOpen)
	{
		bool isTraOpen = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		if (isTraOpen)
		{
		/*	ImGui::Columns(2);
			if (ImGuiProperty<cd::Transform>("Transform", pLightComponent->GetTransform()))
			{
	
			}*/
		}

		ImGui::Columns(1);

		bool isLitSetOpen = ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_DefaultOpen);
		if (isLitSetOpen)
		{
			ImGui::Columns(2);
			ImGui::TextUnformatted("Intensity");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			float intensity = pLightComponent->GetIntensity();
			if (ImGui::DragFloat("##Aspect", &intensity))
			{
				pLightComponent->SetIntensity(intensity);
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::TextUnformatted("Range");
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			float range = pLightComponent->GetRange();
			if (ImGui::DragFloat("##Fov", &range))
			{
				pLightComponent->SetRange(range);
			}

		}
		
	}
	ImGui::Columns(1);
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

	ImGuiIO& io = ImGui::GetIO();
	engine::ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
	engine::SceneWorld* pSceneWorld = pImGuiContextInstance->GetSceneWorld();
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