#pragma once

#include "Application/Localization.h"
#include "ThemeColor.h"

#include <inttypes.h>
#include <memory>
#include <string>
#include <vector>

struct ImGuiContext;

namespace engine
{

class ImGuiBaseLayer;

class ImGuiContextInstance
{
public:
	explicit ImGuiContextInstance(uint16_t width, uint16_t height);
	ImGuiContextInstance(const ImGuiContextInstance&) = delete;
	ImGuiContextInstance& operator=(const ImGuiContextInstance&) = delete;
	ImGuiContextInstance(ImGuiContextInstance&&) = default;
	ImGuiContextInstance& operator=(ImGuiContextInstance&&) = default;
	virtual ~ImGuiContextInstance();

	// ImGui can have multiple ImGuiContexts. For example, you want to render ImGui both in engine and editor.
	// This method helps to switch ImGui's current context pointer to this ImGuiContextInstance.
	// Note that if a method needs to call ImGui api, please call this method in the first line
	// unless you really know what you are doing.
	// Update multiple ImGuiContext in different threads is not safe. So please update them both in main thread by correct order.
	void SwitchCurrentContext() const;

	// Static layer means non-moveable, non-dockable.
	void AddStaticLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);

	// Dockable layer means moveable, dockable.
	std::vector<std::unique_ptr<ImGuiBaseLayer>>& GetDockableLayers() { return m_pImGuiDockableLayers; }
	void AddDynamicLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);
	
	void Update();

	void OnResize(uint16_t width, uint16_t height);
	void OnMouseLBDown();
	void OnMouseLBUp();
	void OnMouseRBDown();
	void OnMouseRBUp();
	void OnMouseMBDown();
	void OnMouseMBUp();
	void OnMouseWheel(float offset);
	void OnMouseMove(int32_t x, int32_t y);
	//void OnKeyPress(int32_t keyCode, uint16_t mods);
	//void OnKeyRelease(int32_t keyCode, uint16_t mods);

	void LoadFontFiles(const std::vector<std::string>& ttfFileNames, engine::Language language);
	ThemeColor GetImGuiThemeColor() const { return m_themeColor; }
	void SetImGuiThemeColor(ThemeColor theme);

private:
	void SetImGuiStyles();
	void BeginDockSpace();
	void EndDockSpace();

private:
	ImGuiContext* m_pImGuiContext;
	ThemeColor m_themeColor;

	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiStaticLayers;
	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiDockableLayers;
};

}