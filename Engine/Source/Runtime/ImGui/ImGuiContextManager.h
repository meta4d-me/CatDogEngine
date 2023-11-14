#pragma once

#include "Core/StringCrc.h"

#include <map>
#include <memory>

namespace engine
{

class ImGuiBaseLayer;
class ImGuiContextInstance;

class ImGuiContextManager
{
public:
	using MapNameToImGuiContext = std::map<StringCrc, std::unique_ptr<ImGuiContextInstance>>;

public:
	ImGuiContextManager();
	ImGuiContextManager(const ImGuiContextManager&) = delete;
	ImGuiContextManager& operator=(const ImGuiContextManager&) = delete;
	ImGuiContextManager(ImGuiContextManager&&) = default;
	ImGuiContextManager& operator=(ImGuiContextManager&&) = default;
	~ImGuiContextManager();

	MapNameToImGuiContext& GetAllImGuiContexts() { return m_allImGuiContexts; }
	const MapNameToImGuiContext& GetAllImGuiContexts() const { return m_allImGuiContexts; }
	ImGuiContextInstance* AddImGuiContext(StringCrc nameCrc);
	ImGuiContextInstance* GetImGuiContext(StringCrc nameCrc) const;
	void RemoveImGuiContext(StringCrc nameCrc);

	// Sometimes, we need access one UILayer data but not in the same context.
	// You can register ui layer information here.
	// TODO : unregister at the proper time to avoid dangling pointers.
	void RegisterImGuiLayersFromContext(const ImGuiContextInstance* pContext);
	void RegisterImGuiLayer(StringCrc nameCrc, ImGuiBaseLayer* pLayer);
	ImGuiBaseLayer* GetImGuiLayer(StringCrc nameCrc) const;
	void UnregisterImGuiLayer(StringCrc nameCrc);

private:
	std::map<StringCrc, ImGuiBaseLayer*> m_mapNameCrcToLayers;
	std::map<StringCrc, std::unique_ptr<ImGuiContextInstance>> m_allImGuiContexts;
};

}