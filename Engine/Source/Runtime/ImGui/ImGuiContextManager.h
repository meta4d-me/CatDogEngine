#pragma once

#include "Core/StringCrc.h"

#include <map>
#include <memory>

namespace engine
{

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

private:
	std::map<StringCrc, std::unique_ptr<ImGuiContextInstance>> m_allImGuiContexts;
};

}