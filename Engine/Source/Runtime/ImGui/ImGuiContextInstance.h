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
	explicit ImGuiContextInstance(uint16_t width, uint16_t height, bool enableDock = false);
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

	// Query if m_pImGuiContext is current active context.
	bool IsActive() const;

	// Static layer means non-moveable, non-dockable.
	void AddStaticLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);

	// Dockable layer means moveable, dockable.
	std::vector<std::unique_ptr<ImGuiBaseLayer>>& GetDockableLayers() { return m_pImGuiDockableLayers; }
	void AddDynamicLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);
	
	void Update();

	void OnResize(uint16_t width, uint16_t height);

	void LoadFontFiles(const std::vector<std::string>& ttfFileNames, engine::Language language);
	ThemeColor GetImGuiThemeColor() const { return m_themeColor; }
	void SetImGuiThemeColor(ThemeColor theme);

	void SetWindowPosOffset(float x, float y) { m_windowPosOffsetX = x; m_windowPosOffsetY = y; }

private:
	void AddInputEvent();
	void SetImGuiStyles();
	void BeginDockSpace();
	void EndDockSpace();

private:
	ImGuiContext* m_pImGuiContext;
	ThemeColor m_themeColor;

	float m_windowPosOffsetX = 0.0f;
	float m_windowPosOffsetY = 0.0f;

	bool m_lastMouseLBPressed = false;
	bool m_lastMouseRBPressed = false;
	bool m_lastMouseMBPressed = false;
	float m_lastMouseScrollOffstY = 0.0f;
	float m_lastMousePositionX = 0.0f;
	float m_lastMousePositionY = 0.0f;

	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiStaticLayers;
	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiDockableLayers;
};

}