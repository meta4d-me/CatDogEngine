#include "ImGuiBaseLayer.h"

#include "ECWorld/SceneWorld.h"
#include "ImGui/ImGuiContextInstance.h"
#include "Rendering/RenderContext.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace engine
{

ImGuiBaseLayer::ImGuiBaseLayer(const char* pName)
	: m_pName(pName)
{
	m_id = ImHashStr(pName);
}

ImGuiWindow* ImGuiBaseLayer::GetRootWindow() const
{
	return ImGui::FindWindowByID(m_id);
}

std::pair<float, float> ImGuiBaseLayer::GetRectPosition() const
{
	auto pos = GetRootWindow()->Pos;
	return std::make_pair(pos.x, pos.y);
}

std::pair<float, float> ImGuiBaseLayer::GetRectSize() const
{
	auto size = GetRootWindow()->Size;
	return std::make_pair(size.x, size.y);
}

ImGuiContextInstance* ImGuiBaseLayer::GetImGuiContextInstance() const
{
	return static_cast<engine::ImGuiContextInstance*>(ImGui::GetIO().UserData);
}

RenderContext* ImGuiBaseLayer::GetRenderContext() const
{
	return static_cast<engine::RenderContext*>(ImGui::GetIO().BackendRendererUserData);
}

SceneWorld* ImGuiBaseLayer::GetSceneWorld() const
{
	return GetImGuiContextInstance()->GetSceneWorld();
}

}