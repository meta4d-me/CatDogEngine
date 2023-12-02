#include "ImGuiContextManager.h"

#include "Base/Template.h"
#include "ImGuiBaseLayer.h"
#include "ImGuiContextInstance.h"

#include <imgui/imgui.h>

namespace engine
{

ImGuiContextManager::ImGuiContextManager()
{
}

ImGuiContextManager::~ImGuiContextManager()
{
}

ImGuiContextInstance* ImGuiContextManager::AddImGuiContext(StringCrc nameCrc)
{
	auto itContext = m_allImGuiContexts.find(nameCrc);
	if (itContext != m_allImGuiContexts.end())
	{
		return itContext->second.get();
	}

	auto newContext = std::make_unique<ImGuiContextInstance>();
	auto* pNewContext = newContext.get();
	pNewContext->SetContextManager(this);
	m_allImGuiContexts[nameCrc] = cd::MoveTemp(newContext);
	return pNewContext;
}

ImGuiContextInstance* ImGuiContextManager::GetImGuiContext(StringCrc nameCrc) const
{
	auto itContext = m_allImGuiContexts.find(nameCrc);
	return itContext != m_allImGuiContexts.end() ? itContext->second.get() : nullptr;
}

void ImGuiContextManager::RemoveImGuiContext(StringCrc nameCrc)
{
	m_allImGuiContexts.erase(nameCrc);
}

void ImGuiContextManager::RegisterImGuiLayersFromContext(const ImGuiContextInstance* pContext)
{
	for (const auto& layer : pContext->GetStaticLayers())
	{
		RegisterImGuiLayer(StringCrc(layer->GetName()), layer.get());
	}

	for (const auto& layer : pContext->GetDynamicLayers())
	{
		RegisterImGuiLayer(StringCrc(layer->GetName()), layer.get());
	}
}

void ImGuiContextManager::RegisterImGuiLayer(StringCrc nameCrc, ImGuiBaseLayer* pLayer)
{
	assert(m_mapNameCrcToLayers.find(nameCrc) == m_mapNameCrcToLayers.end());
	m_mapNameCrcToLayers[nameCrc] = pLayer;
}

ImGuiBaseLayer* ImGuiContextManager::GetImGuiLayer(StringCrc nameCrc) const
{
	auto itUILayer = m_mapNameCrcToLayers.find(nameCrc);
	return itUILayer != m_mapNameCrcToLayers.end() ? itUILayer->second : nullptr;
}

void ImGuiContextManager::UnregisterImGuiLayer(StringCrc nameCrc)
{
	m_mapNameCrcToLayers.erase(nameCrc);
}

}