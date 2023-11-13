#include "ImGuiContextManager.h"

#include "Base/Template.h"
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

}