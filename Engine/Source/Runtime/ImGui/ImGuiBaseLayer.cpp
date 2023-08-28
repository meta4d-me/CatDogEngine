#include "ImGuiBaseLayer.h"

#include "ECWorld/SceneWorld.h"
#include "ImGui/ImGuiContextInstance.h"

#include <imgui/imgui.h>

namespace engine
{

ImGuiContextInstance* ImGuiBaseLayer::GetImGuiContextInstance() const
{
	return reinterpret_cast<engine::ImGuiContextInstance*>(ImGui::GetIO().UserData);
}

RenderContext* ImGuiBaseLayer::GetRenderContext() const
{
	return reinterpret_cast<engine::RenderContext*>(ImGui::GetIO().BackendRendererUserData);
}

SceneWorld* ImGuiBaseLayer::GetSceneWorld() const
{
	return GetImGuiContextInstance()->GetSceneWorld();
}

}