#include "ImGuiBaseLayer.h"

#include "ECWorld/SceneWorld.h"
#include "ImGui/ImGuiContextInstance.h"

#include <imgui/imgui.h>

namespace engine
{

ImGuiContextInstance* ImGuiBaseLayer::GetImGuiContextInstance()
{
	return reinterpret_cast<engine::ImGuiContextInstance*>(ImGui::GetIO().UserData);
}

RenderContext* ImGuiBaseLayer::GetRenderContext()
{
	return reinterpret_cast<engine::RenderContext*>(ImGui::GetIO().BackendRendererUserData);
}

SceneWorld* ImGuiBaseLayer::GetSceneWorld()
{
	return GetImGuiContextInstance()->GetSceneWorld();
}

}