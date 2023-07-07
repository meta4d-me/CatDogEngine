#pragma once 

#include "ImGui/ImGuiBaseLayer.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"

#include <imgui/imgui.h>

namespace ImGuiUtils
{

template<typename T>
void Normalize(T& value)
{
	if constexpr (std::is_same<T, cd::Vec2f>())
	{
		float length = std::sqrt(value.x() * value.x() + value.y() * value.y());
		value.x() /= length;
		value.y() /= length;
	}

	else if constexpr (std::is_same<T, cd::Vec3f>())
	{
		float length = std::sqrt(value.x() * value.x() + value.y() * value.y() + value.z() * value.z());
		value.x() /= length;
		value.y() /= length;
		value.z() /= length;
	}

	else if constexpr (std::is_same <T, cd::Vec4f>())
	{
		float length = std::sqrt(value.x() * value.x() + value.y() * value.y() + value.z() * value.z() + value.w() * value.w());
		value.x() /= length;
		value.y() /= length;
		value.z() /= length;
		value.w() /= length;
	}

}

template<typename T>
bool ImGuiProperty(const char* pName, T& value, const std::string unit = {}, const T & minValue = {}, const T & maxValue = {},bool isNorm = false, float speed  = -1.0)
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
		std::string metricName = std::format("%.2f{}", unit);
		float delta = maxValue - minValue;
		float dragSpeed = (speed <= 0.0) ? (cd::Math::IsEqualToZero(delta) ? 1.0f : delta * 0.05f) : speed;
		if (ImGui::DragFloat(labelName.c_str(), &value, dragSpeed, minValue, maxValue, metricName.c_str()))
		{
			dirty = true;
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();
		ImGui::Columns(1);
	}

	else if constexpr (std::is_same<T, cd::Vec2f>() || std::is_same<T, cd::Vec3f>() || std::is_same<T, cd::Vec4f>())
	{
		if (isNorm)
		{
			Normalize(value);
		}
		ImGui::Columns(2);
		ImGui::TextUnformatted(pName);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		std::string labelName = std::format("##{}", pName);
		std::string metricName = std::format("%.2f{}", unit);
		float delta = maxValue.x() - minValue.x();
		float dragSpeed = (speed <= 0.0) ? (cd::Math::IsEqualToZero(delta) ? 1.0f : delta * 0.05f) : speed;
		if constexpr (std::is_same<T, cd::Vec2f>())
		{
			if (ImGui::DragFloat2(labelName.c_str(), value.Begin(), dragSpeed, minValue.x(), maxValue.x(), metricName.c_str()))
			{
				dirty = true;
			}
		}
		else if constexpr (std::is_same<T, cd::Vec3f>())
		{
			if (ImGui::DragFloat3(labelName.c_str(), value.Begin(), dragSpeed, minValue.x(), maxValue.x(), metricName.c_str()))
			{
				dirty = true;
			}
		}
		else if constexpr (std::is_same<T, cd::Vec4f>())
		{	
			if (ImGui::DragFloat4(labelName.c_str(), value.Begin(), dragSpeed, minValue.x(), maxValue.x(), metricName.c_str()))
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
		if (ImGuiProperty<cd::Vec3f>("Rotation", eularAngles,"", cd::Vec3f::Zero(), cd::Vec3f(360.0f)))
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
		ImGui::Checkbox("Uniform", &UniformScaleEnabled);
		engine::TransformComponent::SetUseUniformScale(UniformScaleEnabled);

		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		if (ImGui::DragFloat3("##Scale", scale.Begin(), 0.1f, 0.001f, 999.0f))
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

}