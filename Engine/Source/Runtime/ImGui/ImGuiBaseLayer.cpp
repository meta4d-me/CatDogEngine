#include "ImGuiBaseLayer.h"

#include "ECWorld/SceneWorld.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/ImGuiContextManager.h"
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

std::pair<float, float> ImGuiBaseLayer::GetWorkRectPosition() const
{
	auto pos = GetRootWindow()->ContentRegionRect.Min;
	return std::make_pair(pos.x, pos.y);
}

std::pair<float, float> ImGuiBaseLayer::GetWorkRectSize() const
{
	auto maxPos = GetRootWindow()->ContentRegionRect.Max;
	auto minPos = GetRootWindow()->ContentRegionRect.Min;
	return std::make_pair(maxPos.x - minPos.x, maxPos.y - minPos.y);
}

ImGuiContextInstance* ImGuiBaseLayer::GetImGuiContextInstance() const
{
	return static_cast<engine::ImGuiContextInstance*>(ImGui::GetIO().UserData);
}

ImGuiContextManager* ImGuiBaseLayer::GetImGuiContextManager() const
{
	return GetImGuiContextInstance()->GetContextManager();
}

ImGuiBaseLayer* ImGuiBaseLayer::GetImGuiLayer(StringCrc nameCrc) const
{
	return GetImGuiContextManager()->GetImGuiLayer(nameCrc);
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